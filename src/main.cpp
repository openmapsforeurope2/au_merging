
//BOOST
#include <boost/program_options.hpp>

//EPG
#include <epg/Context.h>
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/TimeTools.h>
#include <epg/params/tools/loadParameters.h>

//OME2
#include <ome2/utils/setTableName.h>

//APP
#include <app/params/ThemeParameters.h>
#include <app/calcul/AuMergingOp.h>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    epg::Context* context = epg::ContextS::getInstance();
    std::string     logDirectory = "";
    std::string     epgParametersFile = "";
    std::string     themeParametersFile = "";
    std::string     dbName = "";
    std::string     suffix = "";
    std::string     countryCode = "";
    std::string     sourceLevel = "";
    std::string     targetLevel = "";
    bool            verbose = true;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("c" , po::value< std::string >(&epgParametersFile)     , "conf file" )
        ("d", po::value< std::string >(&dbName)                 , "data base name" )
        ("s", po::value< std::string >(&suffix)                 , "working table suffix" )
        ("sl", po::value< std::string >(&sourceLevel)           , "source level" )
        ("tl", po::value< std::string >(&targetLevel)           , "target level" )
    ;

    //main log
    std::string     logFileName = "log.txt";
    std::ofstream   logFile( logFileName.c_str() ) ;

    logFile << "[START] " << epg::tools::TimeTools::getTime() << std::endl;

    int returnValue = 0;
    try{
        po::parsed_options parsed = po::command_line_parser(argc, argv)
                                    .options(desc)
                                    .allow_unregistered()
                                    .run();

        po::variables_map vm;
        po::store( parsed, vm );
        po::notify( vm );    

        if ( vm.count( "help" ) ) {
            std::cout << desc << std::endl;
            return 1;
        }

        // Récupérer les arguments libres (non reconnus)
        std::vector<std::string> countries = po::collect_unrecognized(parsed.options, po::include_positional);

        if ( countries.size() != 1 ) {
            std::string mError = "spécifier un et un seul pays en argument";
            IGN_THROW_EXCEPTION(mError);
        }
        countryCode = countries.front();

        //parametres EPG
		context->loadEpgParameters( epgParametersFile );

        //Initialisation du log de prod
        logDirectory = context->getConfigParameters().getValue( LOG_DIRECTORY ).toString();

        //test si le dossier de log existe sinon le creer
        boost::filesystem::path logDir(logDirectory);
        if (!boost::filesystem::is_directory(logDir))
        {
            if (!boost::filesystem::create_directory(logDir))
            {
                std::string mError = "le dossier " + logDirectory + " ne peut être cree";
                IGN_THROW_EXCEPTION(mError);
            }
        }
        
        //repertoire de travail
        context->setLogDirectory( logDirectory );

        //theme parameters
        themeParametersFile = context->getConfigParameters().getValue( THEME_PARAMETER_FILE ).toString();
		app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
        epg::params::tools::loadParams( *themeParameters, themeParametersFile, countryCode );

        //info de connection db
        context->loadEpgParameters( themeParameters->getValue(DB_CONF_FILE).toString() );
        if( dbName != "" )
            context->getConfigParameters().setParameter(DATABASE, ign::data::String(dbName));

        //epg logger
        epg::log::EpgLogger* logger = epg::log::EpgLoggerS::getInstance();
        // logger->setProdOfstream( logDirectory+"/au_merging.log" );
        logger->setDevOfstream( logDirectory+"/au_merging.log" );

        //table de travail
        std::string levelTemplate = "<LEVEL>";
        if ( !suffix.empty() ) {
            std::string tableBaseName = themeParameters->getValue(TARGET_TABLE_BASE).toString();
            size_t pos = tableBaseName.find(levelTemplate);
            if (pos != std::string::npos) {
                tableBaseName.replace(pos, levelTemplate.length(), targetLevel);
            }
            std::string tableName = tableBaseName + "_" + suffix;
            themeParameters->setParameter(TARGET_TABLE, ign::data::String(tableName));
        }

        //source level
        if( sourceLevel == "" ) {
            std::string sourceLevelTag = themeParameters->getValue(LEVEL_SOURCE_TAG).toString();
            size_t pos = sourceLevelTag.find(levelTemplate);
            if (pos != std::string::npos) {
                sourceLevelTag.replace(pos, levelTemplate.length(), targetLevel);
            }

            std::pair<bool, AU_PARAMETERS> foundParam = themeParameters->convertTagToParam(sourceLevelTag);
            if(foundParam.first)
                sourceLevel = themeParameters->getValue(foundParam.second).toString();
        }

        //table source
        std::string sourceTableName = themeParameters->getValue(SOURCE_TABLE_BASE).toString();
        size_t pos = sourceTableName.find(levelTemplate);
        if (pos != std::string::npos) {
            sourceTableName.replace(pos, levelTemplate.length(), sourceLevel);
        }
        themeParameters->setParameter(SOURCE_TABLE, ign::data::String(sourceTableName));

        //DEBUG
        // context->setVerboseDataBaseManager(true);

        //set BDD search path
        context->getDataBaseManager().setSearchPath(themeParameters->getValue(WORKING_SCHEMA).toString());
        ome2::utils::setTableName<app::params::ThemeParametersS>(TARGET_TABLE);
        ome2::utils::setTableName<app::params::ThemeParametersS>(SOURCE_TABLE);
        ome2::utils::setTableName<epg::params::EpgParametersS>(TARGET_BOUNDARY_TABLE);


        logger->log(epg::log::INFO, "[ START AU MERGING PROCESS ] " + epg::tools::TimeTools::getTime());

        //lancement du traitement
        app::calcul::AuMergingOp::Compute(countryCode, verbose);

		logger->log(epg::log::INFO, "[ END AU MERGING PROCESS ] " + epg::tools::TimeTools::getTime());
    }
    catch( ign::Exception &e )
    {
        std::cerr<< e.diagnostic() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.diagnostic()));
        logFile << e.diagnostic() << std::endl;
        returnValue = 1;
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.what()));
        logFile << e.what() << std::endl;
        returnValue = 1;
    }

    logFile << "[END] " << epg::tools::TimeTools::getTime() << std::endl;

    epg::ContextS::kill();
    epg::log::EpgLoggerS::kill();
    epg::log::ShapeLoggerS::kill();
    epg::params::EpgParametersS::kill();
    app::params::ThemeParametersS::kill();;
    
    logFile.close();

    return returnValue ;
}