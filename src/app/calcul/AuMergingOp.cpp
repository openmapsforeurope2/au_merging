//APP
#include <app/calcul/AuMergingOp.h>
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

// SOCLE
#include <ign/geometry/io/WktWriter.h>
#include <ign/geometry/algorithm/SnapOpGeos.h>


namespace app{
namespace calcul{

	///
	///
	///
    void AuMergingOp::compute(
        std::string sourceTable,
        std::string workingTable,
        std::string countryCode, 
        bool verbose) 
    {
        AuMergingOp auMergingOp(sourceTable, countryCode, verbose);
        auMergingOp._compute(workingTable);
    }

    ///
	///
	///
    AuMergingOp::AuMergingOp( std::string sourceTable, std::string countryCode, bool verbose ):
        _countryCode( countryCode ),
        _verbose( verbose )
    {
        _init(sourceTable);
    }

    ///
	///
	///
    AuMergingOp::~AuMergingOp()
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
    void AuMergingOp::_init(std::string sourceTable) 
    {
        //--
        _logger = epg::log::EpgLoggerS::getInstance();

        //--
        epg::Context* context = epg::ContextS::getInstance();

        // epg parameters
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();

        std::string const idName = epgParams.getValue( ID ).toString();
        std::string const geomName = epgParams.getValue( GEOM ).toString();
        
        //--
        _fsAuArea = context->getDataBaseManager().getFeatureStore(sourceTable, idName, geomName);
    
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
    void AuMergingOp::_compute(std::string workingTable)
    {
        epg::Context* context = epg::ContextS::getInstance();

        //epg params
        epg::params::EpgParameters const& epgParams = context->getEpgParameters();

        std::string const idName = epgParams.getValue( ID ).toString();
        std::string const geomName = epgParams.getValue( GEOM ).toString();
        std::string const countryCodeName = epgParams.getValue( COUNTRY_CODE ).toString();

        // app parameters
        params::ThemeParameters* themeParameters = params::ThemeParametersS::getInstance();
        std::string const stepName = themeParameters->getValue( STEP ).toString();
        std::string const stepValue = themeParameters->getValue( STEP_VALUE ).toString();
        double const slimSurfaceWidth = themeParameters->getValue( SLIM_SURFACE_WIDTH ).toDouble();
        double const smallSurfaceArea = themeParameters->getValue( SMALL_SURFACE_AREA ).toDouble();
        double const snapTolerance = themeParameters->getValue( SNAP_TOLERANCE ).toDouble();

        ign::feature::sql::FeatureStorePostgis* fsAu = context->getDataBaseManager().getFeatureStore(workingTable, idName, geomName);
        ign::feature::FeatureIteratorPtr itAu = fsAu->getFeatures(ign::feature::FeatureFilter(countryCodeName+" = '"+_countryCode+"'"));
        // ign::feature::FeatureIteratorPtr itAu = fsAu->getFeatures(ign::feature::FeatureFilter("inspireid in ('760051db-cd11-4aaa-9a60-23a4a80f9120')"));

        //patience
        int numFeatures = epg::sql::tools::numFeatures( *fsAu, ign::feature::FeatureFilter(countryCodeName+" = '"+_countryCode+"'"));
        boost::progress_display display( numFeatures , std::cout, "[ au_merging  % complete ]\n") ;     

        while (itAu->hasNext())
        {
            ++display;
            ign::feature::Feature fAu = itAu->next();
            ign::geometry::MultiPolygon const& fAuGeom = fAu.getGeometry().asMultiPolygon();

            _shapeLogger->writeFeature( "unit_target", fAu );

            if (_verbose) _logger->log(epg::log::DEBUG,fAu.getId());

            ign::feature::FeatureFilter filter( "ST_Intersects("+_fsAuArea->getFeatureType().getDefaultGeometryName()+",ST_SetSRID(ST_GeomFromText('"+fAuGeom.toString()+"'),3035))");
            filter.addAttribute( _fsAuArea->getFeatureType().getIdName() );
            filter.addAttribute( stepName );
            filter.addAttribute( _fsAuArea->getFeatureType().getDefaultGeometryName() );
            ign::feature::FeatureIteratorPtr itAuArea = _fsAuArea->getFeatures(filter);

            bool isModified = false;
            while (itAuArea->hasNext())
            {
                ign::feature::Feature fAuArea = itAuArea->next();
                ign::geometry::MultiPolygon const& fAuAreaGeom = fAuArea.getGeometry().asMultiPolygon();
                double fAuAreaGeomArea = epg::tools::geometry::getArea( fAuAreaGeom );

                ign::geometry::GeometryPtr result( fAuAreaGeom.Intersection( fAuGeom ) );
                double areaInter = epg::tools::geometry::getArea( *result );

                if ( areaInter < fAuAreaGeomArea*0.9 ) continue;

                if ( fAuArea.getAttribute( stepName ).toString() != stepValue ) continue;

                isModified = true;
                break;
            }
            if ( !isModified ) continue;

            
            ign::feature::FeatureIteratorPtr itAuArea2 = _fsAuArea->getFeatures(filter);

            ign::geometry::GeometryPtr mergedGeom ;
            while (itAuArea2->hasNext())
            {
                ign::feature::Feature fAuArea = itAuArea2->next();
                ign::geometry::MultiPolygon const& fAuAreaGeom = fAuArea.getGeometry().asMultiPolygon();
                double fAuAreaGeomArea = epg::tools::geometry::getArea( fAuAreaGeom );

                _shapeLogger->writeFeature( "unit_source", fAuArea );


                if (_verbose) _logger->log(epg::log::DEBUG,fAuArea.getId());

                ign::geometry::GeometryPtr result( fAuAreaGeom.Intersection( fAuGeom ) );
                double areaInter = epg::tools::geometry::getArea( *result );

                if ( areaInter < fAuAreaGeomArea*0.9 ) continue;

                if( !mergedGeom ) {
                    mergedGeom.reset( fAuAreaGeom.clone() );
                    continue;
                }
					
                ign::geometry::GeometryPtr snappedGeomPtr( ign::geometry::algorithm::SnapOpGeos::SnapTo( *mergedGeom, fAuAreaGeom, snapTolerance ) ) ;
                if (!snappedGeomPtr->equals(fAuAreaGeom)) {
                    _logger->log(epg::log::ERROR, "snapped geometry is different from original");

                    ign::feature::Feature fSnapped;
                    fSnapped.setGeometry(*snappedGeomPtr);
                    _shapeLogger->writeFeature( "snapped", fSnapped );
                }
                ign::geometry::GeometryPtr mergedGeomTemp( mergedGeom->Union( *snappedGeomPtr ) ) ; 
                if( !mergedGeomTemp->isPolygon() && !mergedGeomTemp->isMultiPolygon() )
                {
                    _logger->log(epg::log::ERROR, "merged surface is not a polygon");
                    continue;
                }
                mergedGeom.reset( mergedGeomTemp.release() );
            }

            if( mergedGeom->isPolygon() ) mergedGeom.reset( mergedGeom->asPolygon().toMulti() );
            if( !mergedGeom->isMultiPolygon() ) {
                _logger->log(epg::log::ERROR, "merged surface is not a multipolygon");
                continue;
            }

            ign::feature::Feature f = fAu;
            f.setGeometry(*mergedGeom);
            _shapeLogger->writeFeature( "result", f );

            ign::geometry::MultiPolygon const& mergedMp = mergedGeom->asMultiPolygon();
            for ( size_t i = 0 ; i < mergedMp.numGeometries() ; ++i ) {
                for ( size_t j = 0 ; j < mergedMp.polygonN(i).numRings() ; ++j ) {
                    bool errorFound = false;
                    if (epg::tools::geometry::isSlimSurface(mergedMp.polygonN(i).ringN(j), slimSurfaceWidth)) {
                        _logger->log(epg::log::ERROR, "slim surface detected");
                        errorFound = true;
                    }
                    double area = abs(epg::tools::geometry::getArea(mergedMp.polygonN(i).ringN(j)));
                    if (area < smallSurfaceArea) {
                        _logger->log(epg::log::ERROR, "small surface detected");
                        errorFound = true;
                    }
                    if (errorFound) { 
                        ign::feature::Feature f = fAu;
                        f.setGeometry(mergedMp.polygonN(i).ringN(j));
                        _shapeLogger->writeFeature( "small_or_slim_surface", f );
                    }
                }
            }

            fAu.setGeometry( *mergedGeom ) ;
			fsAu->modifyFeature( fAu ) ;
        }
    }

}
}