
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
		_initParameter( STEP, "STEP" );
		_initParameter( STEP_VALUE, "STEP_VALUE" );
		_initParameter( SLIM_SURFACE_WIDTH, "SLIM_SURFACE_WIDTH" );
		_initParameter( SMALL_SURFACE_AREA, "SMALL_SURFACE_AREA" );
		_initParameter( SNAP_TOLERANCE, "SNAP_TOLERANCE" );
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