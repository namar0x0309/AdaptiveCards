// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "pch.h"

#include "AdaptiveRenderContext.h"
#include "InputValue.h"
#include "Util.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::AdaptiveCards::Rendering::WinUI3;
using namespace ABI::AdaptiveCards::ObjectModel::WinUI3;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Xaml;

namespace AdaptiveCards::Rendering::WinUI3
{
    HRESULT AdaptiveRenderContext::RuntimeClassInitialize() noexcept { return S_OK; }

    HRESULT AdaptiveRenderContext::RuntimeClassInitialize(_In_ IAdaptiveHostConfig* hostConfig,
                                                          _In_ IAdaptiveFeatureRegistration* featureRegistration,
                                                          _In_ IAdaptiveElementRendererRegistration* elementRendererRegistration,
                                                          _In_ IAdaptiveActionRendererRegistration* actionRendererRegistration,
                                                          _In_ IAdaptiveCardResourceResolvers* resourceResolvers,
                                                          _In_ IResourceDictionary* overrideDictionary,
                                                          _In_ IResourceDictionary* defaultActionSentimentStyles,
                                                          _In_ RenderedAdaptiveCard* renderResult) noexcept
    try
    {
        m_hostConfig = hostConfig;
        m_featureRegistration = featureRegistration;
        m_elementRendererRegistration = elementRendererRegistration;
        m_actionRendererRegistration = actionRendererRegistration;
        m_resourceResolvers = resourceResolvers;
        m_overrideDictionary = overrideDictionary;
        m_actionSentimentDefaultDictionary = defaultActionSentimentStyles;

        ComPtr<RenderedAdaptiveCard> strongRenderResult = renderResult;
        RETURN_IF_FAILED(strongRenderResult.AsWeak(&m_weakRenderResult));

        RETURN_IF_FAILED(MakeAndInitialize<AdaptiveActionInvoker>(&m_actionInvoker, renderResult));
        RETURN_IF_FAILED(MakeAndInitialize<AdaptiveMediaEventInvoker>(&m_mediaEventInvoker, renderResult));

        return S_OK;
    }
    CATCH_RETURN();

    HRESULT AdaptiveRenderContext::get_HostConfig(_COM_Outptr_ IAdaptiveHostConfig** value)
    {
        return m_hostConfig.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_FeatureRegistration(_COM_Outptr_ IAdaptiveFeatureRegistration** value)
    {
        return m_featureRegistration.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_ElementRenderers(_COM_Outptr_ IAdaptiveElementRendererRegistration** value)
    {
        return m_elementRendererRegistration.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_ActionRenderers(_COM_Outptr_ IAdaptiveActionRendererRegistration** value)
    {
        return m_actionRendererRegistration.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_ActionInvoker(_COM_Outptr_ IAdaptiveActionInvoker** value)
    {
        return m_actionInvoker.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_MediaEventInvoker(_COM_Outptr_ IAdaptiveMediaEventInvoker** value)
    {
        return m_mediaEventInvoker.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_ResourceResolvers(_COM_Outptr_ IAdaptiveCardResourceResolvers** value)
    {
        return m_resourceResolvers.CopyTo(value);
    }

    HRESULT AdaptiveRenderContext::get_OverrideStyles(_COM_Outptr_ IResourceDictionary** overrideDictionary)
    {
        return m_overrideDictionary.CopyTo(overrideDictionary);
    }

    HRESULT AdaptiveRenderContext::get_UserInputs(_COM_Outptr_ IAdaptiveInputs** value)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        return renderResult->get_UserInputs(value);
    }

    HRESULT AdaptiveRenderContext::AddError(ABI::AdaptiveCards::ObjectModel::WinUI3::ErrorStatusCode statusCode, _In_ HSTRING message)
    {
        ComPtr<IAdaptiveErrorFactory> errorActivationFactory;
        RETURN_IF_FAILED(GetActivationFactory(HStringReference(RuntimeClass_AdaptiveCards_ObjectModel_WinUI3_AdaptiveError).Get(),
                                              &errorActivationFactory));

        ComPtr<IAdaptiveError> error;
        RETURN_IF_FAILED(errorActivationFactory->CreateInstance(statusCode, message, &error));

        ComPtr<IVector<ABI::AdaptiveCards::ObjectModel::WinUI3::AdaptiveError*>> errors;
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult->get_Errors(&errors));
        return (errors->Append(error.Detach()));
    }

    HRESULT AdaptiveRenderContext::AddWarning(ABI::AdaptiveCards::ObjectModel::WinUI3::WarningStatusCode statusCode, _In_ HSTRING message)
    {
        ComPtr<IAdaptiveWarningFactory> warningActivationFactory;
        RETURN_IF_FAILED(
            GetActivationFactory(HStringReference(RuntimeClass_AdaptiveCards_ObjectModel_WinUI3_AdaptiveWarning).Get(),
                                 &warningActivationFactory));

        ComPtr<IAdaptiveWarning> warning;
        RETURN_IF_FAILED(warningActivationFactory->CreateInstance(statusCode, message, &warning));

        ComPtr<IVector<ABI::AdaptiveCards::ObjectModel::WinUI3::AdaptiveWarning*>> warnings;
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult->get_Warnings(&warnings));
        return (warnings->Append(warning.Detach()));
    }

    HRESULT AdaptiveRenderContext::AddInlineShowCard(ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveCard* adaptiveCard,
                                                     ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveShowCardAction* showCardAction,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionButtonUIElement,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionOverflowUIElement,
                                                     ABI::Windows::UI::Xaml::IUIElement* showCardUIElement,
                                                     UINT32 primaryButtonIndex,
                                                     ABI::AdaptiveCards::Rendering::WinUI3::IAdaptiveRenderArgs* renderArgs)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        return renderResult->AddInlineShowCard(
            adaptiveCard, showCardAction, actionButtonUIElement, actionOverflowUIElement, showCardUIElement, primaryButtonIndex, renderArgs);
    }

    HRESULT AdaptiveRenderContext::AddInlineShowCard(_In_opt_ IAdaptiveActionSet* actionSet,
                                                     _In_ IAdaptiveShowCardAction* showCardAction,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionButtonUIElement,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionOverflowUIElement,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* showCardUIElement,
                                                     UINT32 primaryButtonIndex,
                                                     _In_ ABI::AdaptiveCards::Rendering::WinUI3::IAdaptiveRenderArgs* renderArgs)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        return renderResult->AddInlineShowCard(
            actionSet, showCardAction, actionButtonUIElement, actionOverflowUIElement, showCardUIElement, primaryButtonIndex, renderArgs);
    }

    HRESULT AdaptiveRenderContext::AddOverflowButton(_In_ ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveActionSet* actionSet,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionUIElement)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        return renderResult->AddOverflowButton(actionSet, actionUIElement);
    }

    HRESULT AdaptiveRenderContext::AddOverflowButton(_In_ ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveCard* actionCard,
                                                     _In_ ABI::Windows::UI::Xaml::IUIElement* actionUIElement)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        return renderResult->AddOverflowButton(actionCard, actionUIElement);
    }

    HRESULT AdaptiveRenderContext::AddInputValue(_In_ IAdaptiveInputValue* inputValue, _In_ IAdaptiveRenderArgs* renderArgs)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult == nullptr ? E_NOINTERFACE : S_OK);
        return renderResult->AddInputValue(inputValue, renderArgs);
    }

    HRESULT AdaptiveRenderContext::LinkSubmitActionToCard(_In_ ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveActionElement* action,
                                                          _In_ ABI::AdaptiveCards::Rendering::WinUI3::IAdaptiveRenderArgs* renderArgs)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult == nullptr ? E_NOINTERFACE : S_OK);
        return renderResult->LinkActionToCard(action, renderArgs);
    }

    HRESULT AdaptiveRenderContext::LinkCardToParent(_In_ ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveCard* card,
                                                    _In_ ABI::AdaptiveCards::Rendering::WinUI3::IAdaptiveRenderArgs* renderArgs)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult == nullptr ? E_NOINTERFACE : S_OK);
        return renderResult->LinkCardToParent(card, renderArgs);
    }

    HRESULT AdaptiveRenderContext::GetInputValue(_In_ ABI::AdaptiveCards::ObjectModel::WinUI3::IAdaptiveInputElement* inputElement,
                                                 _In_ ABI::AdaptiveCards::Rendering::WinUI3::IAdaptiveInputValue** inputValue)
    {
        ComPtr<RenderedAdaptiveCard> renderResult;
        RETURN_IF_FAILED(GetRenderResult(renderResult.GetAddressOf()));
        RETURN_IF_FAILED(renderResult == nullptr ? E_NOINTERFACE : S_OK);
        return renderResult->GetInputValue(inputElement, inputValue);
    }

    HRESULT AdaptiveRenderContext::get_Rtl(_Out_ ABI::Windows::Foundation::IReference<bool>** rtl)
    {
        return m_rtl.CopyTo(rtl);
    }

    HRESULT AdaptiveRenderContext::put_Rtl(ABI::Windows::Foundation::IReference<bool>* rtl)
    {
        m_rtl = rtl;
        return S_OK;
    }

    HRESULT AdaptiveRenderContext::get_TextStyle(
        _Outptr_ ABI::Windows::Foundation::IReference<ABI::AdaptiveCards::ObjectModel::WinUI3::TextStyle>** textStyle)
    {
        return m_textStyle.CopyTo(textStyle);
    }

    HRESULT AdaptiveRenderContext::put_TextStyle(_In_ ABI::Windows::Foundation::IReference<ABI::AdaptiveCards::ObjectModel::WinUI3::TextStyle>* textStyle)
    {
        m_textStyle = textStyle;
        return S_OK;
    }

    HRESULT AdaptiveRenderContext::get_HorizontalContentAlignment(
        _Outptr_ ABI::Windows::Foundation::IReference<ABI::AdaptiveCards::ObjectModel::WinUI3::HAlignment>** horizontalAlignment)
    {
        return m_horizontalAlignment.CopyTo(horizontalAlignment);
    }

    HRESULT AdaptiveRenderContext::put_HorizontalContentAlignment(
        _In_ ABI::Windows::Foundation::IReference<ABI::AdaptiveCards::ObjectModel::WinUI3::HAlignment>* horizontalAlignment)
    {
        m_horizontalAlignment = horizontalAlignment;
        return S_OK;
    }

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Xaml::IResourceDictionary> AdaptiveRenderContext::GetDefaultActionSentimentDictionary()
    {
        return m_actionSentimentDefaultDictionary;
    }

    HRESULT AdaptiveRenderContext::GetRenderResult(AdaptiveCards::Rendering::WinUI3::RenderedAdaptiveCard** renderResult)
    {
        ComPtr<IRenderedAdaptiveCard> strongRenderResult;
        RETURN_IF_FAILED(m_weakRenderResult.As(&strongRenderResult));
        RETURN_IF_FAILED(strongRenderResult == nullptr ? E_FAIL : S_OK);
        ComPtr<RenderedAdaptiveCard> renderResultObject = PeekInnards<RenderedAdaptiveCard>(strongRenderResult);
        RETURN_IF_FAILED(renderResultObject == nullptr ? E_FAIL : S_OK);
        return renderResultObject.CopyTo(renderResult);
    }
}
