
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
		_initParameter( AU_NAME_TABLE, "AU_NAME_TABLE" );
		_initParameter( AU_GEOMETRY_TABLE, "AU_GEOMETRY_TABLE" );
		_initParameter( NATIONAL_LEVEL_NAME, "NATIONAL_LEVEL_NAME" );
		_initParameter( SHN_NAME, "SHN_NAME" );
		_initParameter( SHN_UPPER_NAME, "SHN_UPPER_NAME" );
		
		_initParameter( SLIM_SURFACE_WIDTH, "SLIM_SURFACE_WIDTH" );
		_initParameter( SMALL_SURFACE_AREA, "SMALL_SURFACE_AREA" );
		_initParameter( SNAP_TOLERANCE, "SNAP_TOLERANCE" );

		_initParameter( TARGET_LEVEL_VALUE, "TARGET_LEVEL_VALUE" );
		_initParameter( LEVEL_VALUE_TAG, "LEVEL_VALUE_TAG" );
		_initParameter( LEVEL_5_VALUE, "LEVEL_5_VALUE" );
		_initParameter( LEVEL_4_VALUE, "LEVEL_4_VALUE" );
		_initParameter( LEVEL_3_VALUE, "LEVEL_3_VALUE" );
		_initParameter( LEVEL_2_VALUE, "LEVEL_2_VALUE" );
		_initParameter( LEVEL_1_VALUE, "LEVEL_1_VALUE" );





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