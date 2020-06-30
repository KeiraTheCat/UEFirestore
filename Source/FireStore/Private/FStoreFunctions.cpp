// Fill out your copyright notice in the Description page of Project Settings.


#include "FStoreFunctions.h"
#include "jwt.h"

void UFStoreFunctions::RequestJsonDocument(FString OAuthToken, FString ProjectID, FString DocumentPath, const FStringDelegate& Del)
{
	ResponseDelegate = Del;
	URestHandler* RH;
	RH = NewObject<URestHandler>();
	TMap<FString, FString> HeaderMap;
	HeaderMap.Add("Authorization", "Bearer " + OAuthToken);
	FString urlStr = preparePathString(ProjectID, DocumentPath);
	RH->MyHttpCall("GET", urlStr, HeaderMap,this,&UFStoreFunctions::RecieveJsonDocument,false);
	RH->ConditionalBeginDestroy();
}
void UFStoreFunctions::RecieveJsonDocument(TSharedPtr<FJsonObject> PTR, FString AsStr)
{
	//Firestore uses nested arrays for map fields
	//to access sub arrays declare a ustruct matching the format of your document field see https://stackoverflow.com/questions/30342465/c-nested-json-in-unreal-engine-4
	//make player data struct
	FJsContainer js;
	//remove json container
	FString clippedStr;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&clippedStr);
	FJsonSerializer::Serialize(PTR->GetObjectField("fields").ToSharedRef(), Writer);

	//write to player data
	FJsonObjectConverter::JsonObjectStringToUStruct<FJsContainer>(clippedStr,&js,0, 0);

	//Print result
	UE_LOG(LogTemp, Display, TEXT("%s"),*clippedStr);
	UE_LOG(LogTemp, Display, TEXT("SubJson: %s"), *js.json.stringValue)
	ResponseDelegate.ExecuteIfBound(*js.json.stringValue);
	UE_LOG(LogHttp, Display, TEXT("End Response"));
}
void UFStoreFunctions::WriteJsonDocument(FString OAuthToken, FString ProjectID, FString DocumentPath, FString JString, const FStringDelegate& Del)
{
	ResponseDelegate = Del;
	URestHandler* RH;
	RH = NewObject<URestHandler>();
	TMap<FString, FString> HeaderMap;
	HeaderMap.Add("Authorization", "Bearer " + OAuthToken);
	FString PayloadString = "{\"fields\": {\"json\": {\"stringValue\": \""+JString+"\"}}}";
	FString urlStr = preparePathString(ProjectID, DocumentPath);
	RH->MyHttpCall("PATCH", urlStr, HeaderMap, this, &UFStoreFunctions::RecieveWriteResponse, false,PayloadString);
	RH->ConditionalBeginDestroy();
}

void UFStoreFunctions::RecieveWriteResponse(TSharedPtr<FJsonObject> PTR, FString AsStr)
{
	ResponseDelegate.ExecuteIfBound(AsStr);
	UE_LOG(LogHttp, Display, TEXT("response: %s"), *AsStr);
	UE_LOG(LogHttp, Display, TEXT("End Response"));
}

void UFStoreFunctions::RecieveAccessToken(TSharedPtr<FJsonObject> PTR, FString AsStr)
{
	FString tokString;
	if (PTR->TryGetStringField("access_token", tokString)) {
		ResponseDelegate.ExecuteIfBound(tokString);
	}
	else {
		ResponseDelegate.ExecuteIfBound("false");
	}
	
}

FString UFStoreFunctions::preparePathString(FString ProjectID, FString DocumentPath)
{
	return ("https://firestore.googleapis.com/v1/projects/" + ProjectID + "/databases/(default)/documents/" + DocumentPath);
}

bool UFStoreFunctions::getToken(FString filename, const FStringDelegate& Del)
{
	//file prep
	FString jsonFile = filename;
	const TCHAR* file = *jsonFile;
	FString result;

	//load file
	bool bout = FFileHelper::LoadFileToString(result, file);
	UE_LOG(LogHttp, Display, TEXT("File found: %s"), bout ? TEXT("true") : TEXT("false"));\

	//parse file
	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(result);

	if (FJsonSerializer::Deserialize(JsonReader, fileJson) && fileJson.IsValid())
	{
		FString pkey = fileJson->GetStringField("private_key");

		//prepare jwt token
		auto token = jwt::create()
			.set_issuer(std::string(TCHAR_TO_UTF8(*fileJson->GetStringField("client_email"))))
			.set_type("JWS")
			.set_subject(std::string(TCHAR_TO_UTF8(*fileJson->GetStringField("client_email"))))
			.set_audience("https://oauth2.googleapis.com/token")
			.set_payload_claim("scope", jwt::claim(std::string("https://www.googleapis.com/auth/datastore")))
			.set_issued_at(std::chrono::system_clock::now())
			.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 3600 })
			.sign(jwt::algorithm::rs256{ "secret",std::string(TCHAR_TO_UTF8(*pkey)) });

		ResponseDelegate = Del;
		//prepare http call
		URestHandler* RH;
		RH = NewObject<URestHandler>();
		TMap<FString, FString> HeaderMap;
		FString tokToFString = token.c_str();
		FString PayloadString = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=" + tokToFString;

		//make http call
		RH->MyHttpCall("POST", "https://oauth2.googleapis.com/token", HeaderMap, this, &UFStoreFunctions::RecieveAccessToken, false, PayloadString);
		RH->ConditionalBeginDestroy();
		return true;
	}
	else 
	{
		return false;
	}
}
