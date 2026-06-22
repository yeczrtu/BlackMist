#include "BlackMistProjectSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"

namespace
{
FString GetPluginDefaultText(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (!PropertyHandle.IsValid() || !PropertyHandle->GetProperty())
	{
		return FString();
	}

	const FBlackMistSettings Defaults = UBlackMistProjectSettings::GetPluginDefaultSettings();
	const FProperty* Property = PropertyHandle->GetProperty();
	const void* DefaultValue = Property->ContainerPtrToValuePtr<void>(&Defaults);

	FString DefaultText;
	Property->ExportTextItem_Direct(DefaultText, DefaultValue, nullptr, nullptr, PPF_PropertyWindow);
	return DefaultText;
}

bool IsResetVisible(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	FString CurrentText;
	if (!PropertyHandle.IsValid() || PropertyHandle->GetValueAsFormattedString(CurrentText, PPF_PropertyWindow) != FPropertyAccess::Success)
	{
		return false;
	}

	return CurrentText != GetPluginDefaultText(PropertyHandle);
}

void ResetPropertyToPluginDefault(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (!PropertyHandle.IsValid())
	{
		return;
	}

	PropertyHandle->SetValueFromFormattedString(GetPluginDefaultText(PropertyHandle));

	if (UBlackMistProjectSettings* Settings = GetMutableDefault<UBlackMistProjectSettings>())
	{
		Settings->SaveConfig();
	}
}

FResetToDefaultOverride MakeResetOverride()
{
	return FResetToDefaultOverride::Create(
		FIsResetToDefaultVisible::CreateStatic(&IsResetVisible),
		FResetToDefaultHandler::CreateStatic(&ResetPropertyToPluginDefault));
}

class FBlackMistProjectSettingsCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FBlackMistProjectSettingsCustomization>();
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
		TSharedRef<IPropertyHandle> DefaultSettingsHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UBlackMistProjectSettings, DefaultSettings));
		DetailBuilder.HideProperty(DefaultSettingsHandle);

		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Black Mist"));

		uint32 NumChildren = 0;
		if (DefaultSettingsHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success)
		{
			return;
		}

		for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
		{
			TSharedPtr<IPropertyHandle> ChildHandle = DefaultSettingsHandle->GetChildHandle(ChildIndex);
			if (!ChildHandle.IsValid())
			{
				continue;
			}

			Category.AddProperty(ChildHandle)
				.OverrideResetToDefault(MakeResetOverride());
		}
	}
};
}

class FBlackMistEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.RegisterCustomClassLayout(
			UBlackMistProjectSettings::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FBlackMistProjectSettingsCustomization::MakeInstance));
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditorModule.UnregisterCustomClassLayout(UBlackMistProjectSettings::StaticClass()->GetFName());
			PropertyEditorModule.NotifyCustomizationModuleChanged();
		}
	}
};

IMPLEMENT_MODULE(FBlackMistEditorModule, BlackMistEditor)
