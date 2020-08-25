// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

//
// FinanceCalculator.xaml.h
// Declaration of the FinanceCalculator class
//

#pragma once

#include "Converters/BooleanNegationConverter.h"
#include "Converters/VisibilityNegationConverter.h"
#include "CalcViewModel\Common\AppResourceProvider.h"
#include "Views\FinanceCalculator.g.h"

namespace CalculatorApp
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class FinanceCalculator sealed
	{
	public:
		FinanceCalculator();
        void SetDefaultFocus();   

    private:
        void OnLoaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
        void FinanceCalculationOption_Changed(Platform::Object ^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^ e);
        void OnCopyMenuItemClicked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
        double FutureValue();
        double InterestEarned();
        void CalculateInterestButton_Click(_In_ Platform::Object ^ sender, _In_ Windows::UI::Xaml::RoutedEventArgs ^ e);
        void OnVisualStateChanged(Platform::Object ^ sender, Windows::UI::Xaml::VisualStateChangedEventArgs ^ e);
        void TipGrid_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
        double TotalNoSplit();
        double CalculatedTipsNoSplit();
        double TotalSplit();
        double CalculatedTipsSplit();
        void SplitBillCheckBox_Checked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
        void SplitBillCheckBox_Unchecked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
        void CalculateTipButton_Click(_In_ Platform::Object ^ sender, _In_ Windows::UI::Xaml::RoutedEventArgs ^ e);

        Windows::Foundation::EventRegistrationToken m_financeCalcOptionChangedEventToken;
    };
}
