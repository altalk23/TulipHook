#include "MacosTarget.hpp"

#include <Platform.hpp>
#include <stdexcept>

using namespace tulip::hook;

#if defined(TULIP_HOOK_MACOS)

#include <mach/mach.h>
#include <mach/mach_init.h> /* mach_task_self()     */
#include <mach/mach_port.h>
#include <mach/mach_vm.h> /* mach_vm_*            */
#include <mach/task.h>

Result<> MacosTarget::allocatePage() {
	kern_return_t status;
	mach_vm_address_t ret;

	status = mach_vm_allocate(mach_task_self(), &ret, static_cast<mach_vm_size_t>(0x4000), VM_FLAGS_ANYWHERE);

	if (status != KERN_SUCCESS) {
		return Err("Couldn't allocate page");
	}

	m_allocatedPage = reinterpret_cast<void*>(ret);
	m_currentOffset = 0;
	m_remainingOffset = 0x4000;

	return this->protectMemory(m_allocatedPage, m_remainingOffset, VM_PROT_COPY | VM_PROT_ALL);
}

Result<uint32_t> MacosTarget::getProtection(void* address) {
	kern_return_t status;
	mach_vm_size_t vmsize;
	mach_vm_address_t vmaddress = reinterpret_cast<mach_vm_address_t>(address);
	vm_region_basic_info_data_t info;
	mach_msg_type_number_t infoCount = VM_REGION_BASIC_INFO_COUNT_64;
	mach_port_t object;

	status = mach_vm_region(
		mach_task_self(),
		&vmaddress,
		&vmsize,
		VM_REGION_BASIC_INFO_64,
		reinterpret_cast<vm_region_info_t>(&info),
		&infoCount,
		&object
	);

	if (status != KERN_SUCCESS) {
		return Err("Couldn't get protection");
	}

	return Ok(info.protection);
}

Result<> MacosTarget::protectMemory(void* address, size_t size, uint32_t protection) {
	kern_return_t status;

	status = mach_vm_protect(mach_task_self(), reinterpret_cast<mach_vm_address_t>(address), size, false, protection);

	if (status != KERN_SUCCESS) {
		return Err("Couldn't protect memory");
	}
	return Ok();
}

Result<> MacosTarget::rawWriteMemory(void* destination, void const* source, size_t size) {
	kern_return_t status;

	status = mach_vm_write(
		mach_task_self(),
		reinterpret_cast<mach_vm_address_t>(destination),
		reinterpret_cast<vm_offset_t>(source),
		static_cast<mach_msg_type_number_t>(size)
	);

	if (status != KERN_SUCCESS) {
		return Err("Couldn't write memory");
	}
	return Ok();
}

uint32_t MacosTarget::getMaxProtection() {
	return VM_PROT_COPY | VM_PROT_ALL;
}

MacosTarget& MacosTarget::get() {
	static MacosTarget ret;
	return ret;
}

Result<ks_engine*> MacosTarget::openKeystone() {
	ks_err status;

	status = ks_open(KS_ARCH_X86, KS_MODE_64, &m_keystone);
	ks_option(m_keystone, KS_OPT_SYNTAX, KS_OPT_SYNTAX_NASM);
	if (status != KS_ERR_OK) {
		return Err("Couldn't open keystone");
	}

	return Ok(m_keystone);
}

Result<csh> MacosTarget::openCapstone() {
	cs_err status;

	status = cs_open(CS_ARCH_X86, CS_MODE_64, &m_capstone);
	if (status != CS_ERR_OK) {
		return Err("Couldn't open capstone");
	}

	return Ok(m_capstone);
}

#endif