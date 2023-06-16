#ifndef _APP_PARAMS_THEMEPARAMETERS_H_
#define _APP_PARAMS_THEMEPARAMETERS_H_

//STL
#include <string>

//EPG
#include <epg/params/ParametersT.h>
#include <epg/SingletonT.h>



	enum AU_PARAMETERS{
		STEP,
		STEP_VALUE,
		SLIM_SURFACE_WIDTH,
		SMALL_SURFACE_AREA,
		SNAP_TOLERANCE
	};

namespace app{
namespace params{

	class ThemeParameters : public epg::params::ParametersT< AU_PARAMETERS >
	{
		typedef  epg::params::ParametersT< AU_PARAMETERS > Base;

		public:

			/// \brief
			ThemeParameters();

			/// \brief
			~ThemeParameters();

			/// \brief
			virtual std::string getClassName()const;

	};

	typedef epg::Singleton< ThemeParameters >   ThemeParametersS;

}
}

#endif