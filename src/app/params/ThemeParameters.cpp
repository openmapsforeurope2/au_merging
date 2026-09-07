
//APP
#include <app/params/ThemeParameters.h>

//SOCLE
#include <ign/Exception.h>


namespace app{
namespace params{


	///
	///
	///
	ThemeParameters::ThemeParameters()
	{
		_initParameter( DB_CONF_FILE, "DB_CONF_FILE" );
		_initParameter( WORKING_SCHEMA, "WORKING_SCHEMA" );
		_initParameter( TARGET_TABLE, "TARGET_TABLE" );
		_initParameter( TARGET_TABLE_BASE, "TARGET_TABLE_BASE" );
		_initParameter( SOURCE_TABLE, "SOURCE_TABLE" );
		_initParameter( SOURCE_TABLE_BASE, "SOURCE_TABLE_BASE" );
		_initParameter( SLIM_SURFACE_WIDTH, "SLIM_SURFACE_WIDTH" );
		_initParameter( SMALL_SURFACE_AREA, "SMALL_SURFACE_AREA" );
		_initParameter( SNAP_TOLERANCE, "SNAP_TOLERANCE" );
		_initParameter( LEVEL_SOURCE_TAG, "LEVEL_SOURCE_TAG" );
		_initParameter( LEVEL_5_SOURCE, "LEVEL_5_SOURCE" );
		_initParameter( LEVEL_4_SOURCE, "LEVEL_4_SOURCE" );
		_initParameter( LEVEL_3_SOURCE, "LEVEL_3_SOURCE" );
		_initParameter( LEVEL_2_SOURCE, "LEVEL_2_SOURCE" );
		_initParameter( LEVEL_1_SOURCE, "LEVEL_1_SOURCE" );
	}

	///
	///
	///
	ThemeParameters::~ThemeParameters()
	{
	}

	///
	///
	///
	std::string ThemeParameters::getClassName()const
	{
		return "app::params::ThemeParameters";
	}

}
}