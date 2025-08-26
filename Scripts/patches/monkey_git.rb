# frozen_string_literal: true
require 'fileutils'
require 'pathname'
require 'tmpdir'

begin
  require 'cocoapods-downloader/git'
rescue LoadError
  begin
    require 'cocoapods'
    require 'cocoapods-downloader/git'
  rescue LoadError
  end
end

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
        suffix = (options[:tag] || options[:commit] || options[:branch] || 'HEAD').to_s.tr('/:\\\\', '-')
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
