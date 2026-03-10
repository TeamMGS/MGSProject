/*
 * 파일명 : MGSDebugHelper.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-09
 */

#pragma once

namespace Debug
{
	// 로그 출력
	static void Print(const FString& Msg, const FColor& Color = FColor::MakeRandomColor(), int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, Color, Msg);
			
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}
	
	// float값 로그 출력
	static void Print(const FString& FloatTitle, float FloatValueToPrint, int32 InKey = -1, const FColor& Color = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			const FString FinalMsg = FloatTitle + TEXT(": ") + FString::SanitizeFloat(FloatValueToPrint);
			GEngine->AddOnScreenDebugMessage(InKey, 7.0f, Color, FinalMsg);
			
			UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalMsg);
		}
	}
}
