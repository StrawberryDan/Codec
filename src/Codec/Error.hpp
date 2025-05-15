#pragma once
#include "Strawberry/Core/Types/Variant.hpp"


namespace Strawberry::Codec
{
	struct ErrorFileNotFound
	{
		std::filesystem::path filePath;
	};


	struct ErrorFileNotSupported
	{
		std::filesystem::path filePath;
	};


	struct ErrorEndOfFile {};


	using Error = Core::Variant<
		ErrorFileNotFound,
		ErrorFileNotSupported,
		ErrorEndOfFile>;
}
