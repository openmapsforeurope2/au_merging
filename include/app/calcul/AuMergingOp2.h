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
	class AuMergingOp2 {

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
		ign::feature::sql::FeatureStorePostgis*            _fsAuName;
		//--
		ign::feature::sql::FeatureStorePostgis*            _fsAuGeometry;
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
		AuMergingOp2( std::string countryCode, bool verbose );

		//--
		~AuMergingOp2();

		//--
		void _init();

		//--
		ign::feature::FeatureCollection _getFeaturesByShn(
			ign::feature::sql::FeatureStorePostgis & fs,
			std::string const& shn
		) const;

		//--
		std::set<std::string> _getSourceShns(
			std::string const& targetAuShn
		) const;

		//--
		std::set<std::string> _getTargetShns() const;

		//--
		bool _getFeatureFromBasicTables(
			std::string const& shn,
			ign::feature::Feature & feature,
			bool withGeometry = true
		) const;

		//--
		void _getCommonField(
			ign::feature::Feature const& featSource,
			ign::feature::Feature & featTarget,
			bool withGeometry
		) const;

		//--
		void _compute() const;
    };

}
}

#endif
