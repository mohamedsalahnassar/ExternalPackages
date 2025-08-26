# frozen_string_literal: true
require 'fileutils'
require 'pathname'
begin
  require 'cocoapods'
rescue LoadError
end
module Pod
  module Downloader
    class Cache
      def in_tmpdir(*)
        root = if ENV['CP_GIT_TMP_ROOT'] && !ENV['CP_GIT_TMP_ROOT'].empty?
                 Pathname.new(ENV['CP_GIT_TMP_ROOT'])
               else
                 Pod::Config.instance.cache_root + 'TmpKeep'
               end
        FileUtils.mkdir_p(root)
        ts  = Time.now.strftime('%Y%m%d-%H%M%S')
        dir = root + "#{ts}-#{$$}-#{rand(1_000_000)}"
        FileUtils.mkdir_p(dir)
        yield(dir)
      end
    end
  end
end
