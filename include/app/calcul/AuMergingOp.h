#ifndef _APP_CALCUL_AUMERGINGOP_H_
#define _APP_CALCUL_AUMERGINGOP_H_

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/geometry/SegmentIndexedGeometry.h>


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>
// #include <epg/tools/MultiLineStringTool.h>


namespace app{
namespace calcul{

	class AuMergingOp {

	public:

		/// \brief
		static void compute(
			std::string sourceTable,
			std::string workingTable, 
			std::string countryCode, 
			bool verbose
		);

	private:
		//--
		ign::feature::sql::FeatureStorePostgis*            _fsAuArea;
		//--
		epg::log::EpgLogger*                               _logger;
		//--
		epg::log::ShapeLogger*                             _shapeLogger;
		//--
		std::string                                        _countryCode;
		//--
		bool                                               _verbose;

	private:

		//--
		AuMergingOp( std::string sourceTable, std::string countryCode, bool verbose );

		//--
		~AuMergingOp();

		//--
		void _init(std::string sourceTable);

		//--
		void _compute(std::string workingTable);
    };

}
}

#endif
