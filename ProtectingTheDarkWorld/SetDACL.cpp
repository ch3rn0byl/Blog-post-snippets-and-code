NTSTATUS SetDACL(PDEVICE_OBJECT& DeviceObject)
{
	unsigned int szSize = 0x1000;

	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE hFile = nullptr;

	ACL* pAclStructure = nullptr;
	ACL* pSecurityDescriptor = nullptr;
	
	// Allocate space in the kernel for the ACL struct
	pAclStructure = static_cast<ACL*>(ExAllocatePoolWithTag(PagedPool, szSize, 'Ch3r'));
	if (pAclStructure == NULL)
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed at ExAllocatePoolWithTag\n", __FUNCTION__, __LINE__);
#endif
		return Status;
	}
	RtlZeroMemory(pAclStructure, szSize);

	// Create the ACL
	Status = RtlCreateAcl(pAclStructure, szSize, ACL_REVISION);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Create the ACE to allow Local System
	Status = RtlAddAccessAllowedAce(pAclStructure, ACL_REVISION, FILE_ALL_ACCESS, SeExports->SeLocalSystemSid);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Create an ACE to allow for Administrators
	Status = RtlAddAccessAllowedAce(pAclStructure, ACL_REVISION, FILE_ALL_ACCESS, SeExports->SeAliasAdminsSid);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Allocate space on the kernel for the security descriptor information
	pSecurityDescriptor = static_cast<ACL*>(ExAllocatePoolWithTag(PagedPool, szSize, 'n0by'));
	if (pSecurityDescriptor == NULL)
	{
		Status = STATUS_UNSUCCESSFUL;
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed at ExAllocatePoolWithTag\n", __FUNCTION__, __LINE__);
#endif
		return Status;
	}
	RtlZeroMemory(pSecurityDescriptor, szSize);

	// Create the security descriptor
	Status = RtlCreateSecurityDescriptor(pSecurityDescriptor, ACL_REVISION1);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Set the security descriptor
	Status = RtlSetDaclSecurityDescriptor(pSecurityDescriptor, TRUE, pAclStructure, FALSE);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Check to see if it was set and verify it is
	if (!RtlValidSecurityDescriptor(pSecurityDescriptor))
	{
		Status = STATUS_UNSUCCESSFUL;
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed at RtlValidSecurityDescriptor\n", __FUNCTION__, __LINE__);
#endif 
		goto CleanUpAndExit;
	}

	// The handle is needed to be able to use ZwSecSecurityObject
	Status = ObOpenObjectByPointer(DeviceObject, NULL, NULL, NULL, NULL, NULL, &hFile);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Sanity Check
	if (hFile == NULL)
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Unable to obtain a handle.\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

	// Finally, enforce the DACL!
	Status = ZwSetSecurityObject(hFile, DACL_SECURITY_INFORMATION, pSecurityDescriptor);
	if (!NT_SUCCESS(Status))
	{
#ifndef _DEBUG
		DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif 
		goto CleanUpAndExit;
	}

CleanUpAndExit:
	if (pAclStructure != NULL)
	{
		ExFreePoolWithTag(pAclStructure, 'Ch3r');
		pAclStructure = nullptr;
	}
	if (pSecurityDescriptor != NULL)
	{
		ExFreePoolWithTag(pSecurityDescriptor, 'n0by');
		pSecurityDescriptor = nullptr;
	}
	if (hFile != NULL)
	{
		ZwClose(hFile);
		hFile = nullptr;
	}
	return Status;
}
