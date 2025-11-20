#ifndef _APP_CALCUL_AUMERGINGOP_H_
#define _APP_CALCUL_AUMERGINGOP_H_

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/geometry/SegmentIndexedGeometry.h>


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


namespace app{
namespace calcul{

	/// @brief Classe dédiée à la fusion des surfaces administratives
	/// permettant de dériver la table des unités administratives de niveau n 
	/// depuis la table des unités administratives de niveau n-1
	class AuMergingOp {

	public:

		/// @brief Lance la création des surfaces administratives de niveau n
		/// par dérivation (fusion) des surfaces administratives de niveau n-1
		/// @param countryCode Code pays simple
		/// @param verbose Mode verbeux
		static void Compute( 
			std::string countryCode, 
			bool verbose
		);

	private:
		//--
		ign::feature::sql::FeatureStorePostgis*            _fsTarget;
		//--
		ign::feature::sql::FeatureStorePostgis*            _fsSource;
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
		AuMergingOp( std::string countryCode, bool verbose );

		//--
		~AuMergingOp();

		//--
		void _init();

		//--
		void _compute() const;
    };

}
}

#endif
