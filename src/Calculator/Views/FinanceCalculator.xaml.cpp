// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "CalcViewModel/Common/CopyPasteManager.h"
#include "CalcViewModel/Common/TraceLogger.h"
#include "CalcViewModel/Common/LocalizationService.h"
#include "CalcViewModel/Common/LocalizationStringUtil.h"
#include "FinanceCalculator.xaml.h"
using namespace std;

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

bool SplitBill(false);
auto resourceLoader = AppResourceProvider::GetInstance();

FinanceCalculator::FinanceCalculator()
{
	InitializeComponent();

    CopyMenuItem->Text = AppResourceProvider::GetInstance()->GetResourceString(L"copyMenuItem");

    m_financeCalcOptionChangedEventToken = FinanceCalculationOption->SelectionChanged +=
        ref new SelectionChangedEventHandler(this, &FinanceCalculator::FinanceCalculationOption_Changed);
}

void FinanceCalculator::OnLoaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{

}

void FinanceCalculator::SetDefaultFocus()
{
    FinanceCalculationOption->Focus(::FocusState::Programmatic);
}

double FinanceCalculator::FutureValue()
{
    double CompoundedValue = 0;
    if (Principle->Text != "" && InterestRate->Text != "" && Term->Text != "")
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
        CompoundedValue = (Base * (pow(1 + (InterestDecimal / Frequency), Years * Frequency)));
    }
    else if (Principle->Text == "" || InterestRate->Text == "" || Term->Text == "")
    {
        CompoundedValue = -1;
    }
    return CompoundedValue;
}

double FinanceCalculator::InterestEarned()
{
    std::wstring FutureValueString(FutureValue().ToString()->Data());
    double CompoundValue = std::stod(FutureValueString);
    double Earnings = 0;

    if (Principle->Text != "" && FutureValue() != -1)
    {
        String ^ PrincipleTextbox = Principle->Text;
        std::wstring PrincipleString(PrincipleTextbox->Data());
        double Base = std::stod(PrincipleString);

        Earnings = (CompoundValue - Base);
    }
    else if (Principle->Text == "" && FutureValue() == -1)
    {
        Earnings = -1;
    }

    return Earnings;
}

void FinanceCalculator::CalculateInterestButton_Click(_In_ Object ^ sender, _In_ RoutedEventArgs ^ e)
{
    double Years = 0;

    if (Term->Text != "")
    {
        // Convert Term textbox to double
        if (FinancialTermType->SelectedIndex == 0)
        {
            String ^ TermTextbox = Term->Text;
            std::wstring TermString(TermTextbox->Data());
            Years = std::stoi(TermString);
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
    }
    else if (Term->Text == "")
    {
        Years = -1;
    }

    String ^ YearsString = Years.ToString();
    String ^ EarningsString = InterestEarned().ToString();

    String ^ OverYears = resourceLoader->GetResourceString(L"OverYears");
    String ^ TotalInterestEarned = resourceLoader->GetResourceString(L"TotalInterestEarned");

    double CompoundedValue = FutureValue();
    double Earnings = InterestEarned();

    if (CompoundedValue != -1 && Years != -1 && Earnings != -1)
    {
        CompoundResults->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + CompoundedValue;
        CompoundSecondaryResults->Text = LocalizationStringUtil::GetLocalizedString(OverYears, YearsString) + " "
                                         + LocalizationStringUtil::GetLocalizedString(TotalInterestEarned, EarningsString);
    }
    else if (CompoundedValue == -1 || Years == -1 || Earnings == -1)
    {
        CompoundResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CalculationFailed");
        CompoundSecondaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"FinancialError");
    }

}

void FinanceCalculator::OnCopyMenuItemClicked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    auto result = safe_cast<TextBlock ^>(Finance_ResultsContextMenu->Target);

    CopyPasteManager::CopyToClipboard(result->Text);
}

void FinanceCalculator::FinanceCalculationOption_Changed(Platform::Object ^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^ e)
{
    FindName("TipCalculationGrid");
    FinanceCalculationOption->SelectionChanged -= m_financeCalcOptionChangedEventToken;
}

void FinanceCalculator::OnVisualStateChanged(Platform::Object ^ sender, Windows::UI::Xaml::VisualStateChangedEventArgs ^ e)
{
    TraceLogger::GetInstance()->LogVisualStateChanged(ViewMode::Date, e->NewState->Name, false);
}

// Code for tip calculation
void FinanceCalculator::TipGrid_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

double FinanceCalculator::TotalNoSplit()
{
    double AmountDue = 0;
    if (BillAmount->Text != "" && TipAmount->Text != "")
    {
        // Convert BillAmount to double 
        String ^ BillAmoutTextbox = BillAmount->Text;
        std::wstring BillAmoutString(BillAmoutTextbox->Data());
        double Bill = std::stod(BillAmoutString);

        // Convert TipAmount to double
        String ^ TipAmountTextbox = TipAmount->Text;
        std::wstring TipAmountString(TipAmountTextbox->Data());
        double Tip = std::stod(TipAmountString);
        double TipDecimal = 0;
        if (TipType->SelectedIndex == 0)
        {
            TipDecimal = ((Tip / 100) + 1.00);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipDecimal = (Tip + 1.00);
        }

        AmountDue = Bill * TipDecimal;
    }
    else if (BillAmount->Text == "" || TipAmount->Text == "")
    {
        AmountDue = -1;
    }

    return AmountDue;
}

double FinanceCalculator::CalculatedTipsNoSplit()
{
    double Total = TotalNoSplit();
    double CalculatedTips = 0;
    if (BillAmount->Text != "" && Total != -1)
    {
        // Convert BillAmount to double
        String ^ BillAmoutTextbox = BillAmount->Text;
        std::wstring BillAmoutString(BillAmoutTextbox->Data());
        double Bill = std::stod(BillAmoutString);

        CalculatedTips = (Total - Bill);
    }
    else if (BillAmount->Text == "" || Total == -1)
    {
        CalculatedTips = -1;
    }

    return CalculatedTips;
}

double FinanceCalculator::TotalSplit()
{
    double AmountDue = 0;
    if (BillAmount->Text != "" && TipAmount->Text != "" && SplitBetween->Text != "")
    {
        // Convert BillAmount to double
        String ^ BillAmoutTextbox = BillAmount->Text;
        std::wstring BillAmoutString(BillAmoutTextbox->Data());
        double Bill = std::stod(BillAmoutString);

        // Convert TipAmount to double
        String ^ TipAmountTextbox = TipAmount->Text;
        std::wstring TipAmountString(TipAmountTextbox->Data());
        double Tip = std::stod(TipAmountString);
        double TipDecimal = 0;
        if (TipType->SelectedIndex == 0)
        {
            TipDecimal = ((Tip / 100) + 1.00);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipDecimal = (Tip + 1.00);
        }

        // Convert SplitBetween to double
        String ^ SplitBetweenTextbox = SplitBetween->Text;
        std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
        double People = std::stod(SplitBetweenString);

        AmountDue = (Bill * TipDecimal) / People;
    }
    else if (BillAmount->Text == "" || TipAmount->Text == "" || SplitBetween->Text == "")
    {
        AmountDue = -1;
    }

    return AmountDue;
}

double FinanceCalculator::CalculatedTipsSplit()
{
    double Total = TotalSplit();
    double CalculatedTips = 0;

    if (BillAmount->Text != "" && Total != -1)
    {
        // Convert BillAmount to double
        String ^ BillAmoutTextbox = BillAmount->Text;
        std::wstring BillAmoutString(BillAmoutTextbox->Data());
        double Bill = std::stod(BillAmoutString);

        // Convert SplitBetween to double
        String ^ SplitBetweenTextbox = SplitBetween->Text;
        std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
        double People = std::stod(SplitBetweenString);

        CalculatedTips = (Total - (Bill / People));
    }
    else if (BillAmount->Text == "" && Total == -1)
    {
        CalculatedTips = -1;
    }

    return CalculatedTips;
}

void FinanceCalculator::SplitBillCheckBox_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::SplitBill = true;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

void FinanceCalculator::SplitBillCheckBox_Unchecked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::SplitBill = false;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

void FinanceCalculator::CalculateTipButton_Click(_In_ Object ^ sender, _In_ RoutedEventArgs ^ e)
{
    String ^ Tips = resourceLoader->GetResourceString(L"Tips");
    String ^ TipsPerPerson = resourceLoader->GetResourceString(L"TipsPerPerson");
    String ^ TotalNoSplitResource = resourceLoader->GetResourceString(L"TotalNoSplit");

    String ^ TotalNoSplitString = TotalNoSplit().ToString();
    String ^ TipsNoSplitString = CalculatedTipsNoSplit().ToString();

    String ^ TotalSplitString = TotalSplit().ToString();
    String ^ TipsSplitString = CalculatedTipsSplit().ToString();

    // Convert SplitBetween to double
    String ^ SplitBetweenTextbox = SplitBetween->Text;
    std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
    double People = std::stod(SplitBetweenString);


    if (::SplitBill == false)
    {
        TipsTotalAmout->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + TotalNoSplit().ToString();
        TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(Tips, TipsNoSplitString);
    }
    else if (::SplitBill == true && People >= 2)
    {
        TipsTotalAmout->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CurrencySymbol") + TotalSplit().ToString() + " "
                               + resourceLoader->GetResourceString(L"PerPerson");
        TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsPerPerson, TipsSplitString, TipsNoSplitString) + " "
                                    + LocalizationStringUtil::GetLocalizedString(TotalNoSplitResource, TotalNoSplitString);
    }
    // If the number of people is 1, then don't split the bill
    else if (::SplitBill == true && People == 1)
    {
        TipsTotalAmout->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + TotalNoSplit().ToString();
        TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(Tips, TipsNoSplitString);
    }
}
