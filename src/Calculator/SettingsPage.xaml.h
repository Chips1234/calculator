//
// SettingsPage.xaml.h
// Declaration of the SettingsPage class
//

#pragma once

#include "SettingsPage.g.h"
#include "views/MainPage.xaml.h"

namespace CalculatorApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SettingsPage sealed
	{
	public:
		SettingsPage();
        property MainPage ^ ParentMainPage;

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^ e) override;

    private:
        void Button_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
    };
}
