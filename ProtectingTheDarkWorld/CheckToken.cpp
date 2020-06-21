NTSTATUS DriverCreate(PDEVICE_OBJECT, PIRP Irp)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT pFileObject = NULL;
	PVOID pTokenInformation = NULL;

	PACCESS_TOKEN pAccessToken = NULL;
	PIO_SECURITY_CONTEXT pSecurityContext = NULL;

	PIO_STACK_LOCATION StackLocation = IoGetCurrentIrpStackLocation(Irp);

	if (Irp->RequestorMode == UserMode)
	{
		pFileObject = StackLocation->FileObject;

		if (!pFileObject || !pFileObject->FileName.Length)
		{
			// Grab the Security Context that way we can have access to the token
			pSecurityContext = StackLocation->Parameters.Create.SecurityContext;
			if (pSecurityContext != NULL && pSecurityContext->AccessState != NULL)
			{
				// Get the primary token to check the integrity of it
				pAccessToken = pSecurityContext->AccessState->SubjectSecurityContext.PrimaryToken;
				if (pAccessToken != NULL)
				{
					Status = SeQueryInformationToken(pAccessToken, TokenIntegrityLevel, &pTokenInformation);
					if (!NT_SUCCESS(Status))
					{
#ifndef _DEBUG
						DbgPrint("[%s::%d] Failed with status: 0x%08x\n", __FUNCTION__, __LINE__, Status);
#endif
						goto ExitFailure;
					}

					// Deny access if it's not being ran by an admin or higher
					if (PtrToUlong(pTokenInformation) < SECURITY_MANDATORY_HIGH_RID)
					{
						Status = STATUS_ACCESS_DENIED;
					}
					pTokenInformation = NULL;
				}
			}
		}
		else
		{
			Status = STATUS_ACCESS_DENIED;
		}
	}

ExitFailure:
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

#ifndef _DEBUG
	DbgPrint("[%s::%d] Completed Successfully.\n", __FUNCTION__, __LINE__);
#endif
	return Status;
}