project "hog"
    kind "ConsoleApp"
    location "build"
    language "C++"
    targetdir "build/%{cfg.buildcfg}"
    objdir "build/%{cfg.buildcfg}/"
    targetname "hog"
    --files {"hog_test.cpp", "../Hog/commons/**.h", "../Hog/commons/**.cpp", "../Hog/commons/**.hpp"}
    files {"hog_shock.cpp",}
    includedirs {"../**", "./"}
    include_custom_headers()
    custom_libdirs()
    links "Responses"
    links "PriceAccess"
    links "CallBackManager"
    --links "Models"
    links "Trade"
    links "ConnectorConfig"
    links "MarginLotSetter"
    links "Indicators"
    links "Execution"
    links "Instruments"
    links "Params"
    links "PnLCalculator"
    links "RiskChecker"
    links "Spot"
    links "Stats"
    links "SymbolLoader"
    links "ValidityChecks"
    links "UniverseMaker"
    links "Helpers"
    links "Utils"
    links "TradeInfoDumper"
    link_illuminati_libs()


