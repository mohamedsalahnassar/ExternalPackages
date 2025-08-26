#!/usr/bin/env ruby
# In-place patcher for cocoapods-downloader's git.rb
# **Autonomous detection** of Homebrew/gem install and versions.
require 'fileutils'
require 'open3'

PRINT_PATH = ARGV.delete('--print-path') ? true : false
MODE = ARGV.shift || 'apply'

def sh(cmd)
  out, _ = Open3.capture2e(cmd)
  out.strip
rescue
  ""
end

def pods_version
  v = sh('pod --version')
  v.empty? ? nil : v.strip
end

def brew_prefixes
  list = []
  bp = sh('brew --prefix 2>/dev/null')
  list << bp unless bp.empty?
  list += %w[/opt/homebrew /usr/local]
  list.uniq.select { |p| File.directory?(p) }
end

def find_downloader_git_by_versions
  pv = pods_version
  brew_prefixes.each do |root|
    # Prefer exact CocoaPods version cellar if available
    if pv
      Dir.glob("#{root}/Cellar/cocoapods/#{pv}*/libexec/gems/**/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb") do |p|
        return p if File.exist?(p)
      end
    end
    # Fallback: opt symlink and any downloader version
    Dir.glob("#{root}/opt/cocoapods/libexec/gems/**/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb") do |p|
      return p if File.exist?(p)
    end
    # Last resort: any cellar version
    Dir.glob("#{root}/Cellar/cocoapods/*/libexec/gems/**/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb").sort.reverse.each do |p|
      return p if File.exist?(p)
    end
  end
  nil
end

def find_downloader_git_by_rubygems
  begin
    require 'rubygems'
    spec = Gem::Specification.find_all_by_name('cocoapods-downloader').max_by { |s| s.version }
    return File.join(spec.gem_dir, 'lib', 'cocoapods-downloader', 'git.rb') if spec
  rescue
  end
  # Common system gem paths
  paths = []
  paths += Dir.glob("/Library/Ruby/Gems/*/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb")
  paths += Dir.glob("/System/Library/Frameworks/Ruby.framework/Versions/*/usr/lib/ruby/gems/*/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb")
  begin
    default_dir = sh('ruby -e "print Gem.default_dir"')
    unless default_dir.empty?
      paths += Dir.glob("#{default_dir}/gems/cocoapods-downloader-*/lib/cocoapods-downloader/git.rb")
    end
  rescue
  end
  paths.uniq.sort_by { |p| File.mtime(p) rescue Time.at(0) }.reverse.first
end

def locate_target
  # 1) explicit override
  if ENV['CP_DL_GIT_RB'] && !ENV['CP_DL_GIT_RB'].empty?
    path = ENV['CP_DL_GIT_RB']
    return path if File.exist?(path)
    abort "CP_DL_GIT_RB points to non-existent path: #{path}"
  end
  # 2) Homebrew using versions
  path = find_downloader_git_by_versions
  return path if path
  # 3) RubyGems fallback
  path = find_downloader_git_by_rubygems
  return path if path
  abort "Could not locate cocoapods-downloader git.rb automatically."
end

PATCH_START = "# MONKEY_GIT_KEEP_PATCH START"
PATCH_END   = "# MONKEY_GIT_KEEP_PATCH END"

def snippet
  <<-'RUBY'

# MONKEY_GIT_KEEP_PATCH START
require 'fileutils'
require 'pathname'
require 'tmpdir'
module Pod
  module Downloader
    module GitKeepPatch
      def clone(*args_in)
        force_head    = args_in[0] unless args_in[0].nil?
        shallow_clone = args_in[1] unless args_in[1].nil?
        force_head    = false if force_head.nil?
        shallow_clone = true  if shallow_clone.nil?
        shallow_clone = false if ENV['CP_GIT_SHALLOW'] == '0'
        root = if ENV['CP_GIT_TMP_ROOT'] && !ENV['CP_GIT_TMP_ROOT'].empty?
                 Pathname.new(ENV['CP_GIT_TMP_ROOT'])
               else
                 Pathname.new(Dir.tmpdir) + 'CocoaPodsGitKeep'
               end
        FileUtils.mkdir_p(root)
        repo   = File.basename(url.to_s, '.git')
        suffix = (options[:tag] || options[:commit] || options[:branch] || 'HEAD').to_s.tr('/:\\\\','-')
        base   = "#{repo}-#{suffix}"
        path   = root + base
        i=0; while File.exist?(path.to_s); i+=1; path = root + "#{base}-#{i}"; end
        singleton_class.class_eval do
          define_method(:target_path) { path.to_s }
        end
        super(force_head, shallow_clone)
      end
    end
  end
end
unless Pod::Downloader::Git.ancestors.include?(Pod::Downloader::GitKeepPatch)
  Pod::Downloader::Git.prepend(Pod::Downloader::GitKeepPatch)
end
# MONKEY_GIT_KEEP_PATCH END
RUBY
end

target = locate_target
backup = target + '.bak.keep_tmp_git'

case MODE
when 'apply'
  original = File.read(target)
  if original.include?(PATCH_START)
    puts "Already patched: #{target}"
    puts target if PRINT_PATH
    exit 0
  end
  FileUtils.cp(target, backup) unless File.exist?(backup)
  File.open(target, 'a'){|f| f.write(snippet) }
  puts "Patched: #{target} (backup at #{backup})"
  puts target if PRINT_PATH
when 'restore'
  if File.exist?(backup)
    FileUtils.mv(backup, target, force: true)
    puts "Restored: #{target}"
    puts target if PRINT_PATH
  else
    puts "No backup found at #{backup}"
    puts target if PRINT_PATH
  end
when 'detect'
  puts locate_target
else
  abort "Unknown mode: #{MODE} (use apply|restore|detect)"
end
