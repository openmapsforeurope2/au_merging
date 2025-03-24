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
		/// @param sourceTable Table source à dériver (unités administratives de niveau n-1)
		/// @param workingTable Table de travail dans laquelle sont créées les unités administratives de niveau n
		/// @param countryCode Code pays simple
		/// @param verbose Mode verbeux
		static void Compute(
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
