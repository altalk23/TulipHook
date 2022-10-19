#include <platform/MacosConvention.hpp>
#include <Platform.hpp>

using namespace tulip::hook;

#if defined(TULIP_HOOK_MACOS)

DefaultConvention::~DefaultConvention() {}

std::string DefaultConvention::generateFromDefault(AbstractFunction const& function) {
	return "ret";
}
std::string DefaultConvention::generateToDefault(AbstractFunction const& function) {
	return "";
}

#endif