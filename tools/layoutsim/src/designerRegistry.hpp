/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: tools/layoutsim/src/designerRegistry.hpp
 * Description: Registry for mapping string IDs to iGfxWidget instances.
 */

#ifndef DESIGNER_REGISTRY_HPP
#define DESIGNER_REGISTRY_HPP

#include <map>
#include <string>
#include "iGfxWidget.hpp"

class DesignerRegistry {
private:
    std::map<std::string, iGfxWidget*> widgets;

public:
    static DesignerRegistry& getInstance() {
        static DesignerRegistry instance;
        return instance;
    }

    void registerWidget(const std::string& id, iGfxWidget* widget) {
        widgets[id] = widget;
    }

    iGfxWidget* getWidget(const std::string& id) {
        auto it = widgets.find(id);
        if (it != widgets.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::map<std::string, iGfxWidget*>& getAllWidgets() const {
        return widgets;
    }

    void clear() {
        widgets.clear();
    }
};

#endif // DESIGNER_REGISTRY_HPP
