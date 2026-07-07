#pragma once

#include "Templates/SharedPointer.h"

class ISlateStyle;

/** Slate Style used by the Game Editor */
class FGameEditorStyle
{
public:

	static void Initialize();
	
	static void Shutdown();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:

	static TSharedRef<class FSlateStyleSet> Create();

	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
