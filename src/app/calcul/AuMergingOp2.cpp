//APP
#include <app/calcul/AuMergingOp2.h>
#include <app/params/ThemeParameters.h>

//BOOST
#include <boost/progress.hpp>

//EPG
#include <epg/Context.h>
#include <epg/params/EpgParameters.h>
#include <epg/sql/tools/numFeatures.h>
#include <epg/tools/TimeTools.h>
#include <epg/tools/geometry/getArea.h>
#include <epg/tools/geometry/isSlimSurface.h>
#include <ome2/feature/sql/NotDestroyedTools.h>

// SOCLE
#include <ign/geometry/io/WktWriter.h>
#include <ign/geometry/algorithm/SnapOpGeos.h>


namespace app{
namespace calcul{

	///
	///
	///
    void AuMergingOp2::Compute(
        std::string countryCode,
        bool verbose
    ) {
        AuMergingOp2 auMergingOp(countryCode, verbose);
        auMergingOp._compute();
    }

    ///
	///
	///
    AuMergingOp2::AuMergingOp2( std::string countryCode, bool verbose ):
        _countryCode( countryCode ),
        _verbose( verbose )
    {
        _init();
    }

    ///
	///
	///
    AuMergingOp2::~AuMergingOp2()
    {
        _shapeLogger->closeShape( "unit_source" );
        _shapeLogger->closeShape( "result" );
        _shapeLogger->closeShape( "small_or_slim_surface" );
        _shapeLogger->closeShape( "unit_target" );
        _shapeLogger->closeShape( "snapped" );

        epg::log::ShapeLoggerS::kill();
    }

    ///
	///
	///
    void AuMergingOp2::_init() 
    {
        //--
        _logger = epg::log::EpgLoggerS::getInstance();

        //--
        epg::Context* context = epg::ContextS::getInstance();

        // epg parameters
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();
        std::string const idName = epgParams.getValue( ID ).toString();
        std::string const geomName = epgParams.getValue( GEOM ).toString();

        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const sourceTable = themeParameters->getValue( SOURCE_TABLE ).toString();
        std::string const targetTable = themeParameters->getValue( TARGET_TABLE ).toString();
        std::string const auNameTable = themeParameters->getValue( AU_NAME_TABLE ).toString();
        std::string const auGeometryTable = themeParameters->getValue( AU_GEOMETRY_TABLE ).toString();
        
        //--
        _fsSource = context->getDataBaseManager().getFeatureStore(sourceTable, idName, geomName);

        //--
        _fsTarget = context->getDataBaseManager().getFeatureStore(targetTable, idName, geomName);

        //--
        _fsAuName = context->getDataBaseManager().getFeatureStore(auNameTable, idName, geomName);

        //--
        _fsAuGeometry = context->getDataBaseManager().getFeatureStore(auGeometryTable, idName, geomName);
    
        //--
        _shapeLogger = epg::log::ShapeLoggerS::getInstance();
        _shapeLogger->setDataDirectory(context->getLogDirectory());
        _shapeLogger->addShape( "unit_source", epg::log::ShapeLogger::POLYGON );
        _shapeLogger->addShape( "result", epg::log::ShapeLogger::POLYGON );
        _shapeLogger->addShape( "small_or_slim_surface", epg::log::ShapeLogger::LINESTRING );
        _shapeLogger->addShape( "unit_target", epg::log::ShapeLogger::POLYGON );
        _shapeLogger->addShape( "snapped", epg::log::ShapeLogger::POLYGON );
        
    };

    ///
    ///
    ///
    std::set<std::string> AuMergingOp2::_getSourceShns(
        std::string const& targetAuShn
    ) const {
        epg::Context* context = epg::ContextS::getInstance();

        //epg params
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();
        std::string const idName = epgParams.getValue( ID ).toString();

        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const auNameTable = themeParameters->getValue( AU_NAME_TABLE ).toString();
        std::string const shnName = themeParameters->getValue( SHN_NAME ).toString();
        std::string const shnUpperName = themeParameters->getValue( SHN_UPPER_NAME ).toString();

        std::ostringstream ss;
        ss << "SELECT " << shnName
           << " FROM " << auNameTable
           << " WHERE " << shnUpperName << "='" << targetAuShn << "'"
           << " AND NOT gcms_detruit";

        ign::sql::SqlResultSetPtr resultPtr = context->getDataBaseManager().getConnection()->query( ss.str() );

        std::set<std::string> sShn;
		for( size_t i = 0 ; i < resultPtr->size() ; ++i )
			sShn.insert( resultPtr->getFieldValue(i,0).toString() );

        return sShn;
    }

    ///
    ///
    ///
    std::set<std::string> AuMergingOp2::_getTargetShns() const
    {
        epg::Context* context = epg::ContextS::getInstance();

        //epg params
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();
        std::string const countryCodeName = epgParams.getValue( COUNTRY_CODE ).toString();

        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const auNameTable = themeParameters->getValue( AU_NAME_TABLE ).toString();
        std::string const shnName = themeParameters->getValue( SHN_NAME ).toString();
        std::string const nationalLevelName = themeParameters->getValue( NATIONAL_LEVEL_NAME ).toString();
        std::string const targetLevelValue = themeParameters->getValue( TARGET_LEVEL_VALUE ).toString();

        std::ostringstream ss;
        ss << "SELECT " << shnName
           << " FROM " << auNameTable
           << " WHERE " << nationalLevelName << "='" << targetLevelValue << "'"
           << " AND " << countryCodeName <<  "='" << _countryCode << "'"
           << " AND NOT gcms_detruit";

        ign::sql::SqlResultSetPtr resultPtr = context->getDataBaseManager().getConnection()->query( ss.str() );

        std::set<std::string> sId;
		for( size_t i = 0 ; i < resultPtr->size() ; ++i )
			sId.insert( resultPtr->getFieldValue(i,0).toString() );

        return sId;
    }

    ///
    ///
    ///
    ign::feature::FeatureCollection AuMergingOp2::_getFeaturesByShn(
        ign::feature::sql::FeatureStorePostgis & fs,
        std::string const& shn
    ) const {
        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const shnName = themeParameters->getValue( SHN_NAME ).toString();

        ign::feature::FeatureFilter filter(shnName + " = '" + shn + "'");

        ign::feature::FeatureIteratorPtr it = ome2::feature::sql::NotDestroyedTools::GetFeatures(fs, filter);

        ign::feature::FeatureCollection featColl;

        while (it->hasNext())
        {
            featColl.push_back(it->next());
        }

        return featColl;
    }

    ///
	///
	///
    bool AuMergingOp2::_getFeatureFromBasicTables(
        std::string const& shn,
        ign::feature::Feature & feature,
        bool withGeometry
    ) const {
        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const shnName = themeParameters->getValue( SHN_NAME ).toString();

        {
            ign::feature::FeatureCollection fCollection = _getFeaturesByShn(*_fsAuName, shn);

            if ( fCollection.size() == 0 ) {
                _logger->log(epg::log::ERROR, "No administrative unit found in name table with shn : " + shn);
                return false;
            }
            if ( fCollection.size() > 1 ) {
                _logger->log(epg::log::ERROR, "Several administrative units in name table have the same shn : " + shn);
                return false;
            }
            _getCommonField( fCollection.front(), feature, !withGeometry);
        }

        if (withGeometry) 
        {
            ign::feature::FeatureCollection fCollection = _getFeaturesByShn(*_fsAuGeometry, shn);

            if ( fCollection.size() == 0 ) {
                _logger->log(epg::log::ERROR, "No administrative unit found in geometry table with shn : " + shn);
                return false;
            }
            if ( fCollection.size() > 1 ) {
                _logger->log(epg::log::ERROR, "Several administrative units in geometry table have the same shn : " + shn);
                return false;
            }
            _getCommonField( fCollection.front(), feature, withGeometry); 
        }

        return true;
    }

    ///
	///
	///
    void AuMergingOp2::_getCommonField(
        ign::feature::Feature const& featSource,
        ign::feature::Feature & featTarget,
        bool withGeometry
    ) const {
        ign::feature::FeatureType fType = featTarget.getFeatureType();

        std::vector<std::string> vAttrNames;
        fType.getAttributeNames(vAttrNames);

        for (size_t i = 0; i < vAttrNames.size(); ++i)
        {
            std::string const& attrName = vAttrNames[i];

            if( attrName == fType.getIdName() )
                continue;

            if( !withGeometry && attrName == fType.getDefaultGeometryName() )
                continue;

            ign::data::Variant targetValue = featTarget.getAttribute(attrName);

            if ( targetValue.isNull() && featSource.hasAttribute(attrName) ) {
                featTarget.setAttribute(attrName, featSource.getAttribute(attrName));
            }
        }
    }

    ///
	///
	///
    void AuMergingOp2::_compute() const
    {
        epg::Context* context = epg::ContextS::getInstance();

        //epg params
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();
        std::string const idName = epgParams.getValue( ID ).toString();
        std::string const geomName = epgParams.getValue( GEOM ).toString();
        std::string const countryCodeName = epgParams.getValue( COUNTRY_CODE ).toString();

        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const shnName = themeParameters->getValue( SHN_NAME ).toString();
        double const slimSurfaceWidth = themeParameters->getValue( SLIM_SURFACE_WIDTH ).toDouble();
        double const smallSurfaceArea = themeParameters->getValue( SMALL_SURFACE_AREA ).toDouble();
        double const snapTolerance = themeParameters->getValue( SNAP_TOLERANCE ).toDouble();

        //--
        std::set<std::string> sShns = _getTargetShns();
        boost::progress_display display( sShns.size() , std::cout, "[ au_merging % complete ]\n");

        for (std::set<std::string>::const_iterator sit = sShns.begin() ; sit != sShns.end() ; ++sit, ++display)
        {
            //DEBUG
            // _logger->log(epg::log::DEBUG, ">>>>" + *sit);
            // if( *sit != "FR938320XG000")
            //     continue;

            std::set<std::string> sSourceShns = _getSourceShns(*sit);

            ign::geometry::GeometryPtr mergedGeom ;
            for (std::set<std::string>::const_iterator sit2 =  sSourceShns.begin() ; sit2 != sSourceShns.end() ; ++sit2)
            {
                //DEBUG
                // _logger->log(epg::log::DEBUG, ">>>>>>>>>>>>>>" + *sit2);
                // if( *sit2 == "FR938320XG112") {
                //     bool test = true;
                // }

                ign::feature::FeatureCollection fCollection = _getFeaturesByShn(*_fsSource, *sit2);

                // if( fCollection.size() > 1 ) {
                //     _logger->log(epg::log::ERROR, "Several administrative units have the same shn : " + *sit2);
                //     continue;
                // }

                // ign::feature::Feature fAuSourceRAM = _fsSource->newFeature();
                // ign::feature::Feature* fAuSource = &fAuSourceRAM;
                // if( fCollection.size() == 1 )
                //     fAuSource = &fCollection.front();
                // else if ( !_getFeatureFromBasicTables(*sit2, fAuSourceRAM) ) 
                //     continue;

                if( fCollection.size() == 0 ) {
                    fCollection.push_back(_fsSource->newFeature());
                    if ( !_getFeatureFromBasicTables(*sit2, fCollection.front()) )
                        continue;
                }

                //DEBUG
                // _logger->log(epg::log::DEBUG, fAuSource->getGeometry().toString());

                for ( size_t i = 0 ; i < fCollection.size() ; ++i ) 
                {
                    ign::geometry::MultiPolygon const& featGeom = fCollection[i].getGeometry().asMultiPolygon();

                    if( !mergedGeom ) {
                        mergedGeom.reset( featGeom.clone() );
                        continue;
                    }

                    ign::geometry::GeometryPtr snappedGeomPtr( ign::geometry::algorithm::SnapOpGeos::SnapTo( *mergedGeom, featGeom, snapTolerance ) ) ;
                    if (!snappedGeomPtr->equals(featGeom)) {
                        _logger->log(epg::log::ERROR, "Snapped geometry is different from original for object shn : " + *sit2);

                        ign::feature::Feature fSnapped;
                        fSnapped.setGeometry(*snappedGeomPtr);
                        _shapeLogger->writeFeature( "snapped", fSnapped );
                    }

                    ign::geometry::GeometryPtr mergedGeomTemp( mergedGeom->Union( *snappedGeomPtr ) ) ;

                    //DEBUG
                    // _logger->log(epg::log::DEBUG, "<<<<<<<<<<<<<UNION>>>>>>>>>>>>>>>");
                    // // _logger->log(epg::log::DEBUG, mergedGeom->toString());
                    // _logger->log(epg::log::DEBUG, "<<<<<<<<<<<<<>>>>>>>>>>>>>>>");

                    if( !mergedGeomTemp->isPolygon() && !mergedGeomTemp->isMultiPolygon() )
                    {
                        _logger->log(epg::log::ERROR, "Union is not a polygon with object shn : " + *sit2);
                        continue;
                    }
                    mergedGeom.reset( mergedGeomTemp.release() );
                }
            }

            if( !mergedGeom || mergedGeom->isNull() ) {
                _logger->log(epg::log::ERROR, "Merged surface is null for object with shn : " + *sit);
                continue;
            }
            if( mergedGeom->isPolygon() ) mergedGeom.reset( mergedGeom->asPolygon().toMulti() );
            if( !mergedGeom->isMultiPolygon() ) {
                _logger->log(epg::log::ERROR, "Merged surface is not a multipolygon for object with shn : " + *sit);
                continue;
            }

            ign::feature::Feature fTarget = _fsTarget->newFeature();
            _getFeatureFromBasicTables(*sit, fTarget, false);
            fTarget.setGeometry(*mergedGeom);
            _shapeLogger->writeFeature( "result", fTarget );

            ign::geometry::MultiPolygon const& mergedMp = mergedGeom->asMultiPolygon();
            for ( size_t i = 0 ; i < mergedMp.numGeometries() ; ++i )
            {
                for ( size_t j = 0 ; j < mergedMp.polygonN(i).numRings() ; ++j )
                {
                    bool errorFound = false;
                    if (epg::tools::geometry::isSlimSurface(mergedMp.polygonN(i).ringN(j), slimSurfaceWidth)) {
                        _logger->log(epg::log::WARN, "Slim surface detected : " + mergedMp.polygonN(i).ringN(j).toString());
                        errorFound = true;
                    }
                    double area = abs(epg::tools::geometry::getArea(mergedMp.polygonN(i).ringN(j)));
                    if (area < smallSurfaceArea) {
                        _logger->log(epg::log::WARN, "Small surface detected : " + mergedMp.polygonN(i).ringN(j).toString());
                        errorFound = true;
                    }
                    if (errorFound) {
                        ign::feature::Feature featLog;
                        featLog.setGeometry(mergedMp.polygonN(i).ringN(j));
                        _shapeLogger->writeFeature( "small_or_slim_surface", featLog );
                    }
                }
            }
            //DEBUG
            // _logger->log(epg::log::DEBUG, "pouet");
            // _logger->log(epg::log::DEBUG, fTarget.getGeometry().toString());

            _fsTarget->createFeature( fTarget ) ;
        }
    }
}
}