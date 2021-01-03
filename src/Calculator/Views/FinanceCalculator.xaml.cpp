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

// Note that "Bill Amount" refers to the total, before tips. "Total" refers to the total, including tips 
// "-1" is returned when there is a error in calculation (e.g. a field is not filled in)

bool IsFindPrinciple(false);
bool IsFindBillAmount(false);
bool IsSplitBill(false);
auto resourceLoader = AppResourceProvider::GetInstance();

FinanceCalculator::FinanceCalculator()
{
	InitializeComponent();

    CopyMenuItem->Text = resourceLoader->GetResourceString(L"copyMenuItem");

    m_financeCalcOptionChangedEventToken = FinanceCalculationOption->SelectionChanged +=
        ref new SelectionChangedEventHandler(this, &FinanceCalculator::FinanceCalculationOption_Changed);

    FindFutureValue->IsChecked = true;
}

void FinanceCalculator::OnLoaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{

}

void FinanceCalculator::SetDefaultFocus()
{
    FinanceCalculationOption->Focus(::FocusState::Programmatic);
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
    TraceLogger::GetInstance()->LogVisualStateChanged(ViewMode::Finance, e->NewState->Name, false);
}

void FinanceCalculator::FindPrinciple_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    PrincipleAndFutureValue->Header = resourceLoader->GetResourceString(L"PrincipleHeader");
    ResultLabel->Text = resourceLoader->GetResourceString(L"FutureValueLabel");
    if (PrincipleAndFutureValue->Text != "" && InterestRate->Text != "" && Term->Text != "" && Compounded->Text != "")
    {
        PrincipleAndFutureValue->Text = FutureValueOrPrincipleValue().ToString();
        CompoundResults->Text = "";
        CompoundSecondaryResults->Text = "";
    }
    else
    {
        PrincipleAndFutureValue->Text = "";
        CompoundResults->Text = "";
        CompoundSecondaryResults->Text = "";
    }
    ::IsFindPrinciple = true;
}

void FinanceCalculator::FindFutureValue_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    PrincipleAndFutureValue->Header = resourceLoader->GetResourceString(L"FutureValueHeader");
    ResultLabel->Text = resourceLoader->GetResourceString(L"PrincipleLabel");
    if (PrincipleAndFutureValue->Text != "" && InterestRate->Text != "" && Term->Text != "" && Compounded->Text != "")
    {
        PrincipleAndFutureValue->Text = FutureValueOrPrincipleValue().ToString();
        CompoundResults->Text = "";
        CompoundSecondaryResults->Text = "";
    }
    else
    {
        PrincipleAndFutureValue->Text = "";
        CompoundResults->Text = "";
        CompoundSecondaryResults->Text = "";
    }
    ::IsFindPrinciple = false;
}

double FinanceCalculator::FutureValueOrPrincipleValue()
{
    double CompoundedValue = 0;
    if (PrincipleAndFutureValue->Text != "" && InterestRate->Text != "" && Term->Text != "")
    {
        // Convert Baserate textbox to double
        String ^ PrincipleTextbox = PrincipleAndFutureValue->Text;
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

        if (::IsFindPrinciple == true)
        {
            // Equation for calculating the future value (Formula: BaseRate *(1 + (Interest[decimal] / Frequency))^(Years * Frequency))
            CompoundedValue = (Base * (pow(1 + (InterestDecimal / Frequency), Years * Frequency)));
        }
        else if (::IsFindPrinciple == false)
        {
            CompoundedValue = (Base / (pow(1 + (InterestDecimal / Frequency), Years * Frequency)));
        }
    }
    else if (PrincipleAndFutureValue->Text == "" || InterestRate->Text == "" || Term->Text == "")
    {
        CompoundedValue = -1;
    }
    return CompoundedValue;
}

double FinanceCalculator::InterestEarned()
{
    std::wstring FutureValueString(FutureValueOrPrincipleValue().ToString()->Data());
    double CompoundValue = std::stod(FutureValueString);
    double Earnings = 0;

    if (PrincipleAndFutureValue->Text != "" && FutureValueOrPrincipleValue() != -1)
    {
        String ^ PrincipleTextbox = PrincipleAndFutureValue->Text;
        std::wstring PrincipleString(PrincipleTextbox->Data());
        double Base = std::stod(PrincipleString);

        if (::IsFindPrinciple == false)
        {
            Earnings = (Base - CompoundValue);
        }
        else if (::IsFindPrinciple == true)
        {
            Earnings = (CompoundValue - Base);
        }
    }
    else if (PrincipleAndFutureValue->Text == "" && FutureValueOrPrincipleValue() == -1)
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

    double CompoundedValue = FutureValueOrPrincipleValue();
    double Earnings = InterestEarned();

    if (CompoundedValue != -1 && Years != -1 && Earnings != -1)
    {
        if (::IsFindPrinciple == false)
        {
            CompoundResults->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + CompoundedValue;
            CompoundSecondaryResults->Text = LocalizationStringUtil::GetLocalizedString(OverYears, YearsString) + " "
                                         + LocalizationStringUtil::GetLocalizedString(TotalInterestEarned, EarningsString);
        }
        else if (::IsFindPrinciple == true)
        {
            CompoundResults->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + CompoundedValue;
            CompoundSecondaryResults->Text = LocalizationStringUtil::GetLocalizedString(OverYears, YearsString) + " "
                                             + LocalizationStringUtil::GetLocalizedString(TotalInterestEarned, EarningsString);
        }
    }
    else if (CompoundedValue == -1 || Years == -1 || Earnings == -1)
    {
        CompoundResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CalculationFailed");
        CompoundSecondaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"FinancialError");
    }

}

// Code for tip calculation
void FinanceCalculator::TipGrid_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    SplitBetween->Text = "1";

    FindBillTotal->IsChecked = true;
}

void FinanceCalculator::FindBillTotal_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    BillAmountOrTotal->Header = resourceLoader->GetResourceString(L"BillAmount");
    TipsResultLabel->Text = resourceLoader->GetResourceString(L"Total");

    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
    {
        if (::IsSplitBill == false)
        {
            BillAmountOrTotal->Text = TotalNoSplit().ToString();
        }
        else
        {
            BillAmountOrTotal->Text = TotalSplit().ToString();
        }
        TipsTotalAmount->Text = "";
        TipsSecodaryResults->Text = "";
    }
    else
    {
        BillAmountOrTotal->Text = "";
        TipsTotalAmount->Text = "";
        TipsSecodaryResults->Text = "";
    }

    ::IsFindBillAmount = false;
}

void FinanceCalculator::FindBillAmount_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    BillAmountOrTotal->Header = resourceLoader->GetResourceString(L"TotalAmount");
    TipsResultLabel->Text = resourceLoader->GetResourceString(L"Amount");

    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
    {
        BillAmountOrTotal->Text = TotalNoSplit().ToString();

        TipsTotalAmount->Text = "";
        TipsSecodaryResults->Text = "";
    }
    else
    {
        BillAmountOrTotal->Text = "";
        CompoundResults->Text = "";
        CompoundSecondaryResults->Text = "";
    }

    ::IsFindBillAmount = true;
}

double FinanceCalculator::TotalNoSplit()
{
    double AmountDueOrTotal = 0;

    // Convert BillAmount to double
    String ^ BillAmoutTextbox = BillAmountOrTotal->Text;
    std::wstring BillAmoutString(BillAmoutTextbox->Data());
    double Bill = std::stod(BillAmoutString);

    // Convert TipAmount to double
    String ^ TipAmountTextbox = TipAmount->Text;
    std::wstring TipAmountString(TipAmountTextbox->Data());
    double Tip = std::stod(TipAmountString);

    double TipDecimal = 0;

    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
    {

        if (TipType->SelectedIndex == 0)
        {
            TipDecimal = ((Tip / 100) + 1.00);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipDecimal = (Tip + 1.00);
        }

        if (::IsFindBillAmount == false)
        {
            AmountDueOrTotal = (Bill * TipDecimal);
        }
        else if (::IsFindBillAmount == true)
        {
            AmountDueOrTotal = (Bill / TipDecimal);
        }
    }
    else if (BillAmountOrTotal->Text == "" || TipAmount->Text == "")
    {
        AmountDueOrTotal = -1;
    }

    return AmountDueOrTotal;
}

double FinanceCalculator::CalculatedTipsNoSplit()
{
    double AmountDueOrTotal = TotalNoSplit();
    double CalculatedTips = 0;

    // Convert BillAmount to double
    String ^ BillAmoutTextbox = BillAmountOrTotal->Text;
    std::wstring BillAmoutString(BillAmoutTextbox->Data());
    double Bill = std::stod(BillAmoutString);

    // Find the total
    if (::IsFindBillAmount == false)
    {
        if (BillAmountOrTotal->Text != "" && AmountDueOrTotal != -1)
        {
            CalculatedTips = (AmountDueOrTotal - Bill);
        }
        else if (BillAmountOrTotal->Text == "" && AmountDueOrTotal == -1)
        {
            CalculatedTips = -1;
        }
    }
    // Find bill amount before tips
    else if (::IsFindBillAmount == true)
    {
        if (BillAmountOrTotal->Text != "" && AmountDueOrTotal != -1)
        {
            CalculatedTips = (Bill - AmountDueOrTotal);
        }
        else if (BillAmountOrTotal->Text == "" && AmountDueOrTotal == -1)
        {
            CalculatedTips = -1;
        }
    }

    return CalculatedTips;
}

double FinanceCalculator::TotalSplit()
{
    double AmountDue = 0;
    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "" && SplitBetween->Text != "")
    {
        // Convert BillAmount to double
        String ^ BillAmoutTextbox = BillAmountOrTotal->Text;
        std::wstring BillAmoutString(BillAmoutTextbox->Data());
        double Bill = std::stod(BillAmoutString);

        // Convert TipAmount to double
        String ^ TipAmountTextbox = TipAmount->Text;
        std::wstring TipAmountString(TipAmountTextbox->Data());
        double Tip = std::stod(TipAmountString);

        // Convert SplitBetween to double
        String ^ SplitBetweenTextbox = SplitBetween->Text;
        std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
        double People = std::stod(SplitBetweenString);

        double TipDecimal = 0;

        if (TipType->SelectedIndex == 0)
        {
            TipDecimal = ((Tip / 100) + 1.00);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipDecimal = (Tip + 1.00);
        }

        if (::IsFindBillAmount == false)
        {
            AmountDue = ((Bill * TipDecimal) / People);
        }
        else if (::IsFindBillAmount == true)
        {
            AmountDue = ((Bill / TipDecimal) / People);
        }

        
    }
    else if (BillAmountOrTotal->Text == "" || TipAmount->Text == "" || SplitBetween->Text == "")
    {
        AmountDue = -1;
    }

    return AmountDue;
}

double FinanceCalculator::CalculatedTipsSplit()
{
    double SplittedTotal = TotalSplit();
    double Total = TotalNoSplit();
    double CalculatedTips = 0;

    // Convert BillAmount to double
    String ^ BillAmoutTextbox = BillAmountOrTotal->Text;
    std::wstring BillAmoutString(BillAmoutTextbox->Data());
    double Bill = std::stod(BillAmoutString);

    // Convert SplitBetween to double
    String ^ SplitBetweenTextbox = SplitBetween->Text;
    std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
    double People = std::stod(SplitBetweenString);

    // Find the total
    if (::IsFindBillAmount == false)
    {
        if (BillAmountOrTotal->Text != "" && SplittedTotal != -1)
        {
            CalculatedTips = (SplittedTotal - (Bill / People));
        }
        else if (BillAmountOrTotal->Text == "" && SplittedTotal == -1)
        {
            CalculatedTips = -1;
        }
    }
    // Find the amount before tips
    else if (::IsFindBillAmount == true)
    {
        if (BillAmountOrTotal->Text != "" && Total != -1)
        {
            CalculatedTips = ((Bill - Total) / People);
        }
        else if (BillAmountOrTotal->Text == "" && Total == -1)
        {
            CalculatedTips = -1;
        }
    }

    return CalculatedTips;
}

void FinanceCalculator::SplitBillCheckBox_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::IsSplitBill = true;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Visible;

    SplitBetween->Text = "";
}

void FinanceCalculator::SplitBillCheckBox_Unchecked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::IsSplitBill = false;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

    SplitBetween->Text = "1";
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

    double People = 0;
    if (SplitBetween->Text != "")
    {
        // Convert SplitBetween to double
        String ^ SplitBetweenTextbox = SplitBetween->Text;
        std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
        People = std::stod(SplitBetweenString);
    }
    else if (SplitBetween->Text == "")
    {
        People = -1;
    }


    if (::IsSplitBill == false)
    {
        if (TotalNoSplit() != -1 && CalculatedTipsNoSplit() != -1)
        {
            TipsTotalAmount->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + TotalNoSplit().ToString();
            TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(Tips, TipsNoSplitString);
        }
        else if (TotalNoSplit() == -1 || CalculatedTipsNoSplit() == -1)
        {
            TipsTotalAmount->Text = resourceLoader->GetResourceString(L"CalculationFailed");
            TipsSecodaryResults->Text = resourceLoader->GetResourceString(L"FinancialError");
        }
    }
    else if (::IsSplitBill == true)
    {
        if (People >= 2 && TotalSplit() != -1 && CalculatedTipsSplit() != -1 && People != -1)
        {
            TipsTotalAmount->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + TotalSplit().ToString() + " "
                                       + resourceLoader->GetResourceString(L"PerPerson");
            TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsPerPerson, TipsSplitString, TipsNoSplitString) + " "
                                            + LocalizationStringUtil::GetLocalizedString(TotalNoSplitResource, TotalNoSplitString);
        }
        // If the number of people is 1, then don't split the bill
        else if (People == 1 && TotalNoSplit() != -1 && CalculatedTipsNoSplit() != -1)
        {
            TipsTotalAmount->Text = resourceLoader->GetResourceString(L"CurrencySymbol") + TotalNoSplit().ToString();
            TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(Tips, TipsNoSplitString);
        }
        else if (TotalNoSplit() == -1 || CalculatedTipsNoSplit() == -1 || People == -1)
        {
            TipsTotalAmount->Text = resourceLoader->GetResourceString(L"CalculationFailed");
            TipsSecodaryResults->Text = resourceLoader->GetResourceString(L"FinancialError");
        }
    }
}
