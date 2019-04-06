/*
 * Copyright (C) 2006-2018 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "SVGForeignObjectElement.h"

#include "CSSPropertyNames.h"
#include "RenderSVGForeignObject.h"
#include "RenderSVGResource.h"
#include "SVGLengthValue.h"
#include "SVGNames.h"
#include <wtf/Assertions.h>
#include <wtf/IsoMallocInlines.h>
#include <wtf/NeverDestroyed.h>

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(SVGForeignObjectElement);

inline SVGForeignObjectElement::SVGForeignObjectElement(const QualifiedName& tagName, Document& document)
    : SVGGraphicsElement(tagName, document)
    , SVGExternalResourcesRequired(this)
{
    ASSERT(hasTagName(SVGNames::foreignObjectTag));
    registerAttributes();
}

Ref<SVGForeignObjectElement> SVGForeignObjectElement::create(const QualifiedName& tagName, Document& document)
{
    return adoptRef(*new SVGForeignObjectElement(tagName, document));
}

void SVGForeignObjectElement::registerAttributes()
{
    auto& registry = attributeRegistry();
    if (!registry.isEmpty())
        return;
    registry.registerAttribute<SVGNames::xAttr, &SVGForeignObjectElement::m_x>();
    registry.registerAttribute<SVGNames::yAttr, &SVGForeignObjectElement::m_y>();
    registry.registerAttribute<SVGNames::widthAttr, &SVGForeignObjectElement::m_width>();
    registry.registerAttribute<SVGNames::heightAttr, &SVGForeignObjectElement::m_height>();
}

void SVGForeignObjectElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    SVGParsingError parseError = NoError;

    if (name == SVGNames::xAttr)
        m_x.setValue(SVGLengthValue::construct(LengthModeWidth, value, parseError));
    else if (name == SVGNames::yAttr)
        m_y.setValue(SVGLengthValue::construct(LengthModeHeight, value, parseError));
    else if (name == SVGNames::widthAttr)
        m_width.setValue(SVGLengthValue::construct(LengthModeWidth, value, parseError));
    else if (name == SVGNames::heightAttr)
        m_height.setValue(SVGLengthValue::construct(LengthModeHeight, value, parseError));

    reportAttributeParsingError(parseError, name, value);

    SVGGraphicsElement::parseAttribute(name, value);
    SVGExternalResourcesRequired::parseAttribute(name, value);
}

void SVGForeignObjectElement::svgAttributeChanged(const QualifiedName& attrName)
{
    if (attrName == SVGNames::widthAttr || attrName == SVGNames::heightAttr) {
        invalidateSVGPresentationAttributeStyle();
        return;
    }

    if (attrName == SVGNames::xAttr || attrName == SVGNames::yAttr) {
        updateRelativeLengthsInformation();
        if (auto renderer = this->renderer())
            RenderSVGResource::markForLayoutAndParentResourceInvalidation(*renderer);
        return;
    }

    SVGGraphicsElement::svgAttributeChanged(attrName);
    SVGExternalResourcesRequired::svgAttributeChanged(attrName);
}

RenderPtr<RenderElement> SVGForeignObjectElement::createElementRenderer(RenderStyle&& style, const RenderTreePosition&)
{
    return createRenderer<RenderSVGForeignObject>(*this, WTFMove(style));
}

bool SVGForeignObjectElement::childShouldCreateRenderer(const Node& child) const
{
    // Disallow arbitary SVG content. Only allow proper <svg xmlns="svgNS"> subdocuments.
    if (child.isSVGElement())
        return child.hasTagName(SVGNames::svgTag);

    // Skip over SVG rules which disallow non-SVG kids
    return StyledElement::childShouldCreateRenderer(child);
}

bool SVGForeignObjectElement::rendererIsNeeded(const RenderStyle& style)
{
    // Suppress foreignObject renderers in SVG hidden containers.
    // (https://bugs.webkit.org/show_bug.cgi?id=87297)
    // Note that we currently do not support foreignObject instantiation via <use>, hence it is safe
    // to use parentElement() here. If that changes, this method should be updated to use
    // parentOrShadowHostElement() instead.
    auto ancestor = makeRefPtr(parentElement());
    while (ancestor && ancestor->isSVGElement()) {
        if (ancestor->renderer() && ancestor->renderer()->isSVGHiddenContainer())
            return false;

        ancestor = ancestor->parentElement();
    }

    return SVGGraphicsElement::rendererIsNeeded(style);
}

}
