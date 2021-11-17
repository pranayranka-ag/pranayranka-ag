require "ninja"

function include_custom_headers()
    sysincludedirs {"/opt/boost/1.54/include/"}
    includedirs {"/opt/xsd/4.0/include/"}
    includedirs {"/opt/xerces-c/3.1.2/include/"}
    includedirs {"/opt/udis86/1.7.2/include/"}
    includedirs {"/opt/libjit/include/"}
    includedirs {"/opt/protobuf/2.6.1/include/"}
    --includedirs {"/data/misc/include/illuminati/dev_beta/rapidjson/"}
    sysincludedirs {"/data/misc/include/"}
    sysincludedirs {"/data/misc/margin/include/"}
    sysincludedirs {"/data/misc/include/illuminati/dev_beta/"}
    includedirs {"/usr/include/"}
    includedirs {"/home/breddy/Strat_CME/Hog/commons/*"}
    includedirs {"/home/breddy/Strat_CME/Hog/"}
    --includedirs {"/home/breddy/Hog/vola/cpp/include"}
end

function custom_libdirs()
    libdirs {"/opt/boost/1.54/lib/"}
    libdirs {"/opt/xerces-c/3.1.2/lib/"}
    libdirs {"/opt/udis86/1.7.2/lib/"}
    libdirs {"/opt/libjit/lib64/"}
    libdirs {"/opt/protobuf/2.6.1/lib/"}
    libdirs {"/opt/levmar/lib/"}
    libdirs {"/data/misc/bld/dev_beta_83/"}
    libdirs {"/data/misc/ag-options-lib/1.6.0/g++/4.8.3/c++11/lib/release/"}
    libdirs {"/opt/lib/"}
    libdirs {"/data/misc/margin/lib"}
    --libdirs {"/home/breddy/Hog/vola/lib"}
end

function link_illuminati_libs()
    --links "z"
    links "coreapirecovery" 
    links "xsim"
    links "commonutils"
    -- links "networks-newlogger"
    links "syssighandler"
    links "agmargin"
    links "aglogger"
    links "numa"
    -- links "zstd_static"
    links "snappy"
    links "z" 
    links "rt"
    links "boost_program_options"
    links "agoptionslib"
    --links "Volar"
end



workspace "boson"
    configurations {"release"} 

solution "new_trade"
  configurations {"release", "debug", "release_raw","debug_raw", "release_no_log", "debug_no_log"}
  gccprefix "/opt/rh/devtoolset-8/root/bin/"
  configuration {"release"}
  targetsuffix ""
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread  -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"NDEBUG"}
    defines {"__CME__"}
    optimize "Speed"
  
  configuration {"release_no_log"}
  targetsuffix ""
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread  -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"_NO_LOG_MODE"}
    defines {"NDEBUG"}
    defines {"__CME__"}
    optimize "Speed"

  configuration {"release_raw"}
  targetsuffix ".release_raw"
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread  -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    -- defines {"_APPEND_TS"}
    defines {"NDEBUG"}
    defines {"__CME__"}
    optimize "Speed"

configuration { "debug_no_log" }
  targetsuffix ""
    buildoptions { "-std=c++17 -g -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas"} -- -fno-omit-frame-pointer -fsanitize=address" } -- "
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"_NO_LOG_MODE"}
    --defines {"DEBUG"}
    defines {"__CME__"}
    flags {"Symbols"}
    optimize "Off"

  configuration { "debug" }
  targetsuffix ""
    buildoptions { "-std=c++17 -g -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas"} -- -fno-omit-frame-pointer -fsanitize=address" } -- "
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    --defines {"_NO_LOG_MODE"}
    --defines {"DEBUG"}
    defines {"__CME__"}
    flags {"Symbols"}
    optimize "Off"

  configuration { "debug_raw" }
  targetsuffix ".debug_raw"
    buildoptions { "-std=c++17 -g -Wno-deprecated-declarations -Wno-unused-but-set-variable -pthread -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"_APPEND_TS"}
    --defines {"DEBUG"}
    defines {"__CME__"}
    flags {"Symbols"}
    optimize "Off"

    include "Hog/commons"
    include "CME"
