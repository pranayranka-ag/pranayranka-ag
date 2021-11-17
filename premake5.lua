require "ninja"
function include_custom_headers()
    sysincludedirs {"/opt/boost/1.54/include/"}
    includedirs {"/opt/xsd/4.0/include/"}
    includedirs {"/opt/xerces-c/3.1.2/include/"}
    includedirs {"/opt/udis86/1.7.2/include/"}
    includedirs {"/opt/libjit/include/"}
    includedirs {"/opt/protobuf/2.6.1/include/"}
    --includedirs {"/data/misc/include/illuminati/dev_beta/rapidjson/"}
    ------------------------------------------------------------------------
    sysincludedirs {"/data/misc/include/"}
    sysincludedirs {"/data/misc/external/"}
    sysincludedirs {"/data/misc/margin/include/"}
    sysincludedirs {"/data/misc/include/illuminati/dev_beta/"}
    ------------------------------------------------------------------------
    
    
    includedirs {"/usr/include/"}
    includedirs {"$(HOME)/commons/*"}
    sysincludedirs {"/data/misc/ag-options-lib/1.5.0/include/"}
    -- sysincludedirs {"/data/misc/ag-shocklib/1.0.0/include/agshocklib/"}
    -- includedirs {"/home/pranka/work/Strat_Init/Hog/ta_lib/include/ta-lib/*"}
    includedirs {"/home/pranka/work/Strat_Init/Hog/commons/include/armadillo_bits/*"}
    -- includedirs {"/home/pranka/anaconda3/include/"}
    -- includedirs {"/home/pranka/work/Strat_Init/Hog/openblas/include/*"}
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
    -- libdirs {"/data/misc/bld/dev_beta_48/"}
    -- libdirs {"/data/misc/ag-options-lib/1.5.0/g++/4.8.3/c++11/lib/release/"}
    libdirs {"/opt/lib/"}
    libdirs {"/data/misc/margin/lib"}
    libdirs {"/data/misc/ag-options-lib/1.6.0/g++/4.8.3/c++11/lib/release/"}

    -- libdirs {"/home/pranka/work/Strat_Init/Hog/openblas/lib/"}
    -- libdirs {"/home/pranka/work/Strat_Init/Hog/commons/ta_lib/lib/*"}
    libdirs {"/home/pranka/work/Strat_Init/Hog/commons/lib64/"}
    libdirs {"/usr/lib64/"}
    -- libdirs {"/data/misc/"}
    -- libdirs {"/data/misc/ag-shocklib/1.0.0/include/agshocklib/"}
    -- libdirs {"/home/pranka/anaconda3/lib/"}
    -- libdirs {"/home/pranka/work/Strat_Init/Hog/usr/lib64/"}
    --libdirs {"/home/breddy/Hog/vola/lib"}
end
function link_illuminati_libs()
    --links "z"
    links "coreapirecovery" 
    links "xsim"
    links "commonutils"
    -- links "networks-newlogger"
    links "syssighandler"
    links "nsefomargin"
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
    links "pthread"
    links "networks-newlogger"
    links "agoptionslib"
    -- links "blas"
    -- links "lapack"
    -- links "hdf5_cpp"
    -- links "Armadillo"
end
workspace "boson"
    configurations {"release"} 
solution "new_trade"
  configurations {"release", "debug", "release_raw","debug_raw","release_no_log"}
  gccprefix "/opt/rh/devtoolset-8/root/bin/"
  configuration {"release"}
  targetsuffix ""
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -fpermissive -pthread  -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"NDEBUG"}
    --flags {"Symbols"}
    optimize "Speed"
 configuration {"release_no_log"}
  targetsuffix ".no_log"
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -fpermissive -pthread  -Wall -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
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
    optimize "Speed"
  configuration {"release_raw"}
  targetsuffix ".release_raw"
    buildoptions { "-std=c++17 -Wno-deprecated-declarations -fpermissive -pthread  -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "-f
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    defines {"NDEBUG"}
    optimize "Speed"
  configuration { "debug" }
  targetsuffix ""
    buildoptions { "-g -std=c++17 -Wno-deprecated-declarations -w -fpermissive -pthread -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    --defines {"DEBUG"}
    flags {"Symbols"}
    optimize "Off"
  configuration { "debug_raw" }
  targetsuffix ".debug_raw"
    buildoptions { "-g -std=c++17 -Wno-deprecated-declarations -fpermissive -pthread -Wall -Wno-class-memaccess -Wno-stringop-truncation -Wno-switch -Wno-unused-function -Wno-unused-variable -Wno-reorder -Wno-sign-compare -Wno-unknown-pragmas" } -- "
    defines {"__STDC_LIMIT_MACROS"}
    defines {"__STDC_CONSTANT_MACROS"}
    defines {"__STDC_FORMAT_MACROS"}
    defines {"USE_MWMRQ_REQSHM"}
    defines {"USE_MWMRQ_RESPSHM"}
    defines {"USE_MWMRQ_MDSHM"}
    defines {"CONNECTOR_ZEROCOPY"}
    defines {"_SIGNAL_ON_MD_EMPTYQ"}
    --defines {"DEBUG"}
    flags {"Symbols"}
    optimize "Off"
    include "Hog/commons"
    -- include "Hog/commons/Indicators/TaLib"
    include "Strat"