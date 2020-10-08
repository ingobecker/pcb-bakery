require 'ceedling'
Ceedling.load_project

libopencm3_sources = Rake::FileList["**/*.c", "**/*.h"]
file 'vendor/libopencm3/lib/opencm3_stm32f1.a': libopencm3_sources do
  #sh "git submodule update --init"
  sh "make -C vendor/libopencm3"
end

task release: 'vendor/libopencm3/lib/opencm3_stm32f1.a'

namespace :target do

  desc 'flash release into target and drop into gdb session'
  task debug: :release do
    exec <<-GDB.gsub(/\s+/, " ")
        gdb-multiarch
          -ex 'target extended-remote /dev/ttyACM0'
          -x vendor/black_magic_probe_flash.scr
          #{PROJECT_BUILD_RELEASE_ROOT}/#{RELEASE_BUILD_OUTPUT}
          GDB
  end

  desc 'flash release into target'
  task flash: :release do
    exec <<-GDB.gsub(/\s+/, " ")
        gdb-multiarch
          --batch
          -ex 'target extended-remote /dev/ttyACM0'
          -x vendor/black_magic_probe_flash.scr
          -ex kill
          #{PROJECT_BUILD_RELEASE_ROOT}/#{RELEASE_BUILD_OUTPUT}
          GDB
  end
end
