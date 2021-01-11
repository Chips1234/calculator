// This is for the spec

#include "pch.h"
#include "CalcViewModel/Common/CopyPasteManager.h"
#include "CalcViewModel/Common/TraceLogger.h"
#include "CalcViewModel/Common/LocalizationService.h"
#include "CalcViewModel/Common/LocalizationStringUtil.h"
#include "FinanceCalculator.xaml.h"
#include "math.h"

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

    FindBillTotal->IsChecked = true;
    ::IsSplitBill = false;
}

void FinanceCalculator::FindBillTotal_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    BillAmountOrTotal->Header = resourceLoader->GetResourceString(L"BillAmount");
    TipsResultLabel->Text = resourceLoader->GetResourceString(L"Total");

    if (::IsSplitBill == false)
    {
        if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
        {
            BillAmountOrTotal->Text = BillAmount().ToString();
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
        else
        {
            BillAmountOrTotal->Text = "";
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
    }
    else
    {
        if (BillAmountOrTotal->Text != "" && TipAmount->Text != "" && SplitBetween->Text != "")
        {
            String ^ SplitBetweenTextbox = SplitBetween->Text;
            std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
            double People = std::stod(SplitBetweenString);

            BillAmountOrTotal->Text = (BillAmount() * People).ToString();
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
        else
        {
            BillAmountOrTotal->Text = "";
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
    }

    ::IsFindBillAmount = false;
}

void FinanceCalculator::FindBillAmount_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    BillAmountOrTotal->Header = resourceLoader->GetResourceString(L"TotalAmount");
    TipsResultLabel->Text = resourceLoader->GetResourceString(L"Amount");

    if (::IsSplitBill == false)
    {
        if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
        {
            BillAmountOrTotal->Text = BillTotal().ToString();
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
        else
        {
            BillAmountOrTotal->Text = "";
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
    }
    else
    {
        if (BillAmountOrTotal->Text != "" && TipAmount->Text != "" && SplitBetween->Text != "")
        {
            String ^ SplitBetweenTextbox = SplitBetween->Text;
            std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
            double People = std::stod(SplitBetweenString);

            BillAmountOrTotal->Text = (BillTotal() * People).ToString();
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
        else
        {
            BillAmountOrTotal->Text = "";
            TipsTotalAmount->Text = "";
            TipsSecodaryResults->Text = "";
        }
    }

    ::IsFindBillAmount = true;
}

void FinanceCalculator::SplitBillCheckBox_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::IsSplitBill = true;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

void FinanceCalculator::SplitBillCheckBox_Unchecked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    ::IsSplitBill = false;
    SplitBetween->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

double FinanceCalculator::BillTotal()
{
    double Total = 0;
    double Rounded = 0;

    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
    {
        // Convert BillAmoutOrTotal to double
        String ^ BillAmountOrTotalTextbox = BillAmountOrTotal->Text;
        std::wstring BillAmountOrTotalString(BillAmountOrTotalTextbox->Data());
        double Bill = std::stod(BillAmountOrTotalString);

        // Convert TipAmount to double
        String ^ TipAmountTextbox = TipAmount->Text;
        std::wstring TipAmountString(TipAmountTextbox->Data());
        double Tips = std::stod(TipAmountString);

        double TipsDecimal = 0;

        // If the inputed tips value is in percent form, convert it to decimals, else, leave unchanged.
        if (TipType->SelectedIndex == 0)
        {
            TipsDecimal = (Tips / 100);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipsDecimal = Tips;
        }

        if (::IsSplitBill == false)
        {
            Total = (Bill * (1 + TipsDecimal));
        }
        else if (::IsSplitBill == true)
        {
            if (SplitBetween->Text != "")
            {
                String ^ SplitBetweenTextbox = SplitBetween->Text;
                std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
                double People = std::stod(SplitBetweenString);

                Total = ((Bill * (1 + TipsDecimal)) / People);
            }
            else
            {
                Total = -1;
            }
        }

        Rounded = round(Total * 20) / 20;
    }
    else if (BillAmountOrTotal->Text == "" || TipAmount->Text == "" || Total == -1)
    {
        Rounded = -1;
    }

    return Rounded;
}

double FinanceCalculator::TipsAmount()
{
    double Tips = 0;

    if (BillAmountOrTotal->Text != "" && BillTotal() != -1)
    {
        // Convert BillAmoutOrTotal to double
        String ^ BillAmountOrTotalTextbox = BillAmountOrTotal->Text;
        std::wstring BillAmountOrTotalString(BillAmountOrTotalTextbox->Data());
        double Bill = std::stod(BillAmountOrTotalString);

        double Total = BillTotal();
        double BillAmountDouble = BillAmount();

        if (::IsSplitBill == false)
        {
            if (::IsFindBillAmount == false)
            {
                Tips = (Total - Bill);
            }
            else
            {
                Tips = (Bill - BillAmountDouble);
            }
        }
        else
        {
            if (SplitBetween->Text != "")
            {
                String ^ SplitBetweenTextbox = SplitBetween->Text;
                std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
                double People = std::stod(SplitBetweenString);

                if (::IsFindBillAmount == false)
                {
                    Tips = (((Total * People) - Bill) / People);
                }
                else
                {
                    Tips = ((Bill - (BillAmountDouble * People)) / People);
                }
            }
            else
            {
                Tips = -1;
            }
        }
    }
    else if (BillAmountOrTotal->Text == "" || BillTotal() == -1 || Tips == -1)
    {
        Tips = -1;
    }

    return Tips;
}

double FinanceCalculator::BillAmount()
{
    double Amount = 0;
    double Rounded = 0;

    if (BillAmountOrTotal->Text != "" && TipAmount->Text != "")
    {
        // Convert BillAmoutOrTotal to double
        String ^ BillAmountOrTotalTextbox = BillAmountOrTotal->Text;
        std::wstring BillAmountOrTotalString(BillAmountOrTotalTextbox->Data());
        double Total = std::stod(BillAmountOrTotalString);

        // Convert TipAmount to double
        String ^ TipAmountTextbox = TipAmount->Text;
        std::wstring TipAmountString(TipAmountTextbox->Data());
        double Tips = std::stod(TipAmountString);

        double TipsDecimal = 0;

        // If the inputed tips value is in percent form, convert it to decimals, else, leave unchanged.
        if (TipType->SelectedIndex == 0)
        {
            TipsDecimal = (Tips / 100);
        }
        else if (TipType->SelectedIndex == 1)
        {
            TipsDecimal = Tips;
        }

        if (::IsSplitBill == false)
        {
            Amount = (Total / (1 + TipsDecimal));
        }
        else
        {
            String ^ SplitBetweenTextbox = SplitBetween->Text;
            std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
            double People = std::stod(SplitBetweenString);

            Amount = ((Total / (1 + TipsDecimal)) / People);
        }
        Rounded = round(Amount * 20) / 20;
    }
    else
    {
        Rounded = -1;
    }

    return Rounded;
}

void FinanceCalculator::CalculateTipButton_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    String ^ TipsString = resourceLoader->GetResourceString(L"Tips");
    String ^ TotalNoSplitString = resourceLoader->GetResourceString(L"TotalNoSplit");
    String ^ TipsPerPersonString = resourceLoader->GetResourceString(L"TipsPerPerson");

    if (::IsFindBillAmount == false)
    {
        if (::IsSplitBill == false)
        {
            if (BillTotal() != -1 && TipsAmount() != -1)
            {
                TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CurrencySymbol") + BillTotal().ToString();
                TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsString, TipsAmount().ToString());
            }
            else
            {
                TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CalculationFailed");
                TipsSecodaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"FinancialError");
            }
        }
        else
        {
            if (BillTotal() != -1 && TipsAmount() != -1)
            {
                String ^ SplitBetweenTextbox = SplitBetween->Text;
                std::wstring SplitBetweenString(SplitBetweenTextbox->Data());
                double People = std::stod(SplitBetweenString);

                double TotalNoSplitAmount = BillTotal() * People;
                double TipsNoSplit = TipsAmount() * People;

                // If there are less than two people, don't split bill
                if (People >= 2)
                {
                    TipsTotalAmount->Text =
                        resourceLoader->GetResourceString(L"CurrencySymbol") + BillTotal().ToString() + " " + resourceLoader->GetResourceString(L"PerPerson");
                    TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsPerPersonString, TipsAmount().ToString(), TipsNoSplit.ToString())
                                                + " " + LocalizationStringUtil::GetLocalizedString(TotalNoSplitString, TotalNoSplitAmount.ToString());
                }
                else
                {
                    TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CurrencySymbol") + BillTotal().ToString();
                    TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsString, TipsAmount().ToString());
                }
            }
            else
            {
                TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CalculationFailed");
                TipsSecodaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"FinancialError");
            }
        }
    }
    else if (::IsFindBillAmount == true)
    {
        if (BillTotal() != -1 && TipsAmount() != -1)
        {
            TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CurrencySymbol") + BillAmount().ToString();
            TipsSecodaryResults->Text = LocalizationStringUtil::GetLocalizedString(TipsString, TipsAmount().ToString());
        }
        else
        {
            TipsTotalAmount->Text = AppResourceProvider::GetInstance()->GetResourceString(L"CalculationFailed");
            TipsSecodaryResults->Text = AppResourceProvider::GetInstance()->GetResourceString(L"FinancialError");
        }
    }
}
