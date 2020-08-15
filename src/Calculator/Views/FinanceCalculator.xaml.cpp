// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "CalcViewModel/Common/CopyPasteManager.h"
#include "CalcViewModel/Common/TraceLogger.h"
#include "FinanceCalculator.xaml.h"

using namespace CalculatorApp;
using namespace CalculatorApp::Common;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::System::UserProfile;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Automation;
using namespace Windows::UI::Xaml::Automation::Peers;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

FinanceCalculator::FinanceCalculator()
{
	InitializeComponent();

    CopyMenuItem->Text = AppResourceProvider::GetInstance()->GetResourceString(L"copyMenuItem");

    m_financeCalcOptionChangedEventToken = FinanceCalculationOption->SelectionChanged +=
        ref new SelectionChangedEventHandler(this, &FinanceCalculator::FinanceCalculationOption_Changed);
}

void FinanceCalculator::SetDefaultFocus()
{
    FinanceCalculationOption->Focus(::FocusState::Programmatic);
}

double FinanceCalculator::FutureValue()
{
    // Convert Baserate textbox to double
    String ^ PrincipleTextbox = Principle->Text;
    std::wstring PrincipleString(PrincipleTextbox->Data());
    double Base = std::stod(PrincipleString);

    // Convert Interest textbox to double
    double InterestDecimal = 0;
    if (InterestType->SelectedIndex == 0)
    {
        String ^ InterestTextBox = InterestRate->Text;
        std::wstring InterestString(InterestTextBox->Data());
        double Interest = std::stod(InterestString);
        InterestDecimal = (Interest / 100);
    }
    else if (InterestType->SelectedIndex == 1)
    {
        String ^ InterestTextBox = InterestRate->Text;
        std::wstring InterestString(InterestTextBox->Data());
        InterestDecimal = std::stod(InterestString);
    }

    // Convert Term textbox to double. If it is a day, devide the value by 365. If it is a month, 12.
    double Years = 0;
    if (FinancialTermType->SelectedIndex == 0)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        Years = std::stod(TermString);
    }
    else if (FinancialTermType->SelectedIndex == 1)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        double month = std::stod(TermString);
        Years = (month / 12);
    }
    else if (FinancialTermType->SelectedIndex == 2)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        double day = std::stod(TermString);
        Years = (day / 365);
    }

    // Convert how many times it is compounded field to double
    String ^ CompoundedTextBox = Compounded->Text;
    std::wstring CompoundedString(CompoundedTextBox->Data());
    double Frequency = std::stod(CompoundedString);

    // Equation for calculating the future value (Formula: BaseRate *(1 + (Interest[decimal] / Frequency))^(Years * Frequency))
    double CompoundedValue = Base * (pow(1 + (InterestDecimal / Frequency), Years * Frequency));

    return CompoundedValue;
}

double FinanceCalculator::InterestEarned()
{
    double CompoundedValue = FutureValue();

    String ^ PrincipleTextbox = Principle->Text;
    std::wstring PrincipleString(PrincipleTextbox->Data());
    double Base = std::stod(PrincipleString);

    double Earnings = CompoundedValue - Base;

    return Earnings;
}

void FinanceCalculator::CalculateInterestButton_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    // Convert Term textbox to double
    double Years = 0;
    if (FinancialTermType->SelectedIndex == 0)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        Years = std::stod(TermString);
    }
    else if (FinancialTermType->SelectedIndex == 1)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        double month = std::stod(TermString);
        Years = (month / 12);
    }
    else if (FinancialTermType->SelectedIndex == 2)
    {
        String ^ TermTextbox = Term->Text;
        std::wstring TermString(TermTextbox->Data());
        double day = std::stod(TermString);
        Years = (day / 365);
    }

    double CompoundedValue = FutureValue();
    double Earnings = InterestEarned();

        CompoundResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CurrencySymbol") + CompoundedValue.ToString();
        CompoundSecondaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"Over") + " " + Years.ToString() + " "
                                         + AppResourceProvider::GetInstance()->GetResourceString(L"Finance_Years") + ". "
                                         + AppResourceProvider::GetInstance()->GetResourceString(L"TotalInterestEarned")+" "+Earnings.ToString();
}

void CalculatorApp::FinanceCalculator::OnCopyMenuItemClicked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    auto result = safe_cast<TextBlock ^>(Finance_ResultsContextMenu->Target);

    CopyPasteManager::CopyToClipboard(result->Text);
}

void CalculatorApp::FinanceCalculator::TipGrid_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
}

void CalculatorApp::FinanceCalculator::FinanceCalculationOption_Changed(Platform::Object ^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^ e)
{
    FindName("TipCalculationGrid");
    FinanceCalculationOption->SelectionChanged -= m_financeCalcOptionChangedEventToken;
}

void CalculatorApp::FinanceCalculator::OnLoaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
}

void FinanceCalculator::OnVisualStateChanged(Platform::Object ^ sender, Windows::UI::Xaml::VisualStateChangedEventArgs ^ e)
{
    TraceLogger::GetInstance()->LogVisualStateChanged(ViewMode::Date, e->NewState->Name, false);
}
