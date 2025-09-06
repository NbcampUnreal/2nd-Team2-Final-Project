// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHong1, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCheatManager, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSetting, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGimmick, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGimmickUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogInventoryUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogInventoryItem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCraftingUI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogEnvironment, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogDa, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMonster, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHelperBot, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAnimal, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSave, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGM, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogTutorial, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogUtility, Log, All);

#define FUNCTION_TCHAR (ANSI_TO_TCHAR(__FUNCTION__))
#pragma region CR4S_Log

#define CR4S_Log_Explicit(LogCategory, Verbosity, InFunctionName, InLineNumber, Format, ...) \
    UE_LOG(LogCategory, Verbosity, TEXT("[%s::%d] %s"), InFunctionName, InLineNumber, *FString::Printf(Format, ##__VA_ARGS__))

#define CR4S_Log(LogCategory, Verbosity, Format, ...) \
    CR4S_Log_Explicit(LogCategory, Verbosity, FUNCTION_TCHAR, __LINE__, Format, ##__VA_ARGS__)

#pragma endregion

#pragma region CR4S_Validate

#define CR4S_VALIDATE(LogCategory, Expression) \
    [&, OuterFunctionName = FUNCTION_TCHAR, OuterLineNumber = __LINE__]() -> bool { \
        if (!(Expression)) { \
            CR4S_Log_Explicit(LogCategory, Warning, OuterFunctionName, OuterLineNumber, TEXT("Validation Failed: %s"), TEXT(#Expression)); \
            return false; \
        } \
        return true; \
    }()

#pragma endregion


// 함수 시작 부분에 위치시키면 함수의 시작, 종료 시점에 알아서 로그 출력
// 함수들의 호출 스택을 런타임에 로그로 파악하기에 용이
struct FCR4SSimpleScopeLogger
{
    FString FileName;
    int32   LineNumber;
    FName   FunctionName;

    FCR4SSimpleScopeLogger(const TCHAR* InFile, int32 InLine, const TCHAR* InFunction)
        : FileName     ( FPaths::GetCleanFilename(InFile) )
        , LineNumber   ( InLine )
        , FunctionName ( InFunction )
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[%s:%d] [BEGIN] %s"),
            *FileName,
            LineNumber,
            *FunctionName.ToString()
        );
    }

    ~FCR4SSimpleScopeLogger()
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT("[%s:%d] [ END ] %s"),
            *FileName,
            LineNumber,
            *FunctionName.ToString()
        );
    }
};

#define CR4S_SIMPLE_SCOPE_LOG \
FCR4SSimpleScopeLogger CR4SScopeLoggerInstance( \
        ANSI_TO_TCHAR(__FILE__), \
        __LINE__, \
        ANSI_TO_TCHAR(__FUNCTION__) \
    )

#define CR4S_ENSURE_IMPLEMENTATION(Category, Expression, bShouldBreak)           \
    ([&]() -> bool                                                                   \
    {                                                                                   \
        static bool bShouldBreakOnFirst = bShouldBreak;                               \
        if (!(Expression)) \
        { \
            if (bShouldBreakOnFirst) \
            { \
                /* 첫 실패: 지정된 카테고리로 로그 + 브레이크 */                            \
                UE_LOG(                                                               \
                    Category,                                                         \
                    Warning,                                                          \
                    TEXT("[%s:%d] Ensure failed: %s"),                                \
                    TEXT(__FILE__), __LINE__, TEXT(#Expression)                       \
                );                                                                    \
                PLATFORM_BREAK();                                                     \
                bShouldBreakOnFirst = false;                                          \
                return false;                                                         \
            }                                                                         \
            else                                                                      \
            {                                                                         \
                /* 두 번째 이후 실패: 메시지 포함 로그만 */                                \
                UE_LOG(                                                               \
                    Category,                                                         \
                    Warning,                                                          \
                    TEXT("[%s:%d] Ensure failed: %s"),                                \
                    TEXT(__FILE__), __LINE__, TEXT(#Expression)                       \
                );                                                                    \
                return false;                                                         \
            }                                                                         \
        }                                                                             \
        return true;                                                                  \
    }())

#define CR4S_ENSURE(Category, Expression) CR4S_ENSURE_IMPLEMENTATION(Category, Expression, false)
#define CR4S_ENSURE_ONCE(Category, Expression) CR4S_ENSURE_IMPLEMENTATION(Category, Expression, true)