// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireStoreBPLibrary.h"
#include "FireStore.h"
#include "RestHandler.h"

UFireStoreBPLibrary::UFireStoreBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UFireStoreBPLibrary::FireStoreSampleFunction(float Param)
{
	return -1;
}
bool UFireStoreBPLibrary::FireStoreRequest(FString OAUTHToken, FString ProjectID, FString documentPath, const FStringDelegate& Del)
{
	UFStoreFunctions* SF;
	SF = NewObject<UFStoreFunctions>();
	SF->RequestJsonDocument(OAUTHToken, ProjectID, documentPath, Del);
	SF->ConditionalBeginDestroy();
	return true;
}

bool UFireStoreBPLibrary::FireStorePatch(FString OAUTHToken, FString ProjectID, FString documentPath, FString content, const FStringDelegate& Del)
{
	UFStoreFunctions* SF;
	SF = NewObject<UFStoreFunctions>();
	SF->WriteJsonDocument(OAUTHToken, ProjectID, documentPath, content.ReplaceCharWithEscapedChar(),Del);
	SF->ConditionalBeginDestroy();
	return false;
}

void UFireStoreBPLibrary::getAccessToken(FString filename, const FStringDelegate& Del)
{
	UFStoreFunctions* SF;
	SF = NewObject<UFStoreFunctions>();
	SF->getToken(filename,Del);
	SF->ConditionalBeginDestroy();
}

FString UFireStoreBPLibrary::SerializeStructToJson(const FProperty* Struct)
{
	return "meow";
}

