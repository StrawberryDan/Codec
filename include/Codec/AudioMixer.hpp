#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
/// Codec
#include "FilterGraph.hpp"
#include "Frame.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Option.hpp"



namespace Strawberry::Codec
{
	class AudioMixer
			: private FilterGraph
	{

	};
}
