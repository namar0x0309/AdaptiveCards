// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "pch.h"

#include "AdaptiveTextBlockConfig.h"

using namespace Microsoft::WRL;
using namespace ABI::AdaptiveCards::Rendering::WinUI3;

namespace AdaptiveCards::Rendering::WinUI3
{
    HRESULT AdaptiveTextBlockConfig::RuntimeClassInitialize() noexcept
    try
    {
        TextBlockConfig textBlockConfig;
        return RuntimeClassInitialize(textBlockConfig);
    }
    CATCH_RETURN();

    HRESULT AdaptiveTextBlockConfig::RuntimeClassInitialize(TextBlockConfig textBlockConfig) noexcept
    {
        m_headingLevel = textBlockConfig.headingLevel;
        return S_OK;
    }

    HRESULT AdaptiveTextBlockConfig::get_HeadingLevel(_Out_ UINT32* headingLevel)
    {
        *headingLevel = m_headingLevel;
        return S_OK;
    }

    HRESULT AdaptiveTextBlockConfig::put_HeadingLevel(UINT32 headingLevel)
    {
        m_headingLevel = headingLevel;
        return S_OK;
    }
}
