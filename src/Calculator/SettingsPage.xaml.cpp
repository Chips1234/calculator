//
// SettingsPage.xaml.cpp
// Implementation of the SettingsPage class
//

#include "pch.h"
#include "SettingsPage.xaml.h"
#include "views/MainPage.xaml.h"

using namespace CalculatorApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

SettingsPage::SettingsPage()
{
	InitializeComponent();
}

void SettingsPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^ e)
{
    MainPage ^ mainPage = (MainPage ^) e->Parameter;
    if (mainPage != nullptr)
    {
        ParentMainPage = mainPage;
    }
}

void SettingsPage::Button_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    auto rootFrame = dynamic_cast<::Frame ^>(Window::Current->Content);
    rootFrame->GoBack();
}
