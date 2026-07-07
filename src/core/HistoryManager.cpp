#include "core/HistoryManager.h"
#include <algorithm>
#include <iostream>

namespace core {

HistoryManager::HistoryManager(int maxStates)
    : m_MaxStates(maxStates), m_CurrentIndex(-1) {}

HistoryManager::~HistoryManager() {
    Clear();
}

void HistoryManager::Clear() {
    m_History.clear();
    m_ActionNames.clear();
    m_CurrentIndex = -1;
}

void HistoryManager::RecordState(const LayerStack& stack, const std::string& actionName) {
    // If we are in the middle of the history (after undos) and a new action is performed,
    // discard all forward redo history states.
    if (m_CurrentIndex >= 0 && m_CurrentIndex < static_cast<int>(m_History.size()) - 1) {
        m_History.erase(m_History.begin() + m_CurrentIndex + 1, m_History.end());
        m_ActionNames.erase(m_ActionNames.begin() + m_CurrentIndex + 1, m_ActionNames.end());
    }

    // Capture current snapshot
    LayerStackSnapshot snapshot = CreateSnapshot(stack);
    m_History.push_back(std::move(snapshot));
    m_ActionNames.push_back(actionName);

    // Enforce size limit
    if (static_cast<int>(m_History.size()) > m_MaxStates) {
        m_History.erase(m_History.begin());
        m_ActionNames.erase(m_ActionNames.begin());
    }

    m_CurrentIndex = static_cast<int>(m_History.size()) - 1;
}

bool HistoryManager::Undo(LayerStack& stack) {
    if (!CanUndo()) return false;
    
    m_CurrentIndex--;
    RestoreSnapshot(m_History[m_CurrentIndex], stack);
    return true;
}

bool HistoryManager::Redo(LayerStack& stack) {
    if (!CanRedo()) return false;
    
    m_CurrentIndex++;
    RestoreSnapshot(m_History[m_CurrentIndex], stack);
    return true;
}

bool HistoryManager::JumpToState(int index, LayerStack& stack) {
    if (index < 0 || index >= static_cast<int>(m_History.size())) return false;
    
    m_CurrentIndex = index;
    RestoreSnapshot(m_History[m_CurrentIndex], stack);
    return true;
}

LayerStackSnapshot HistoryManager::CreateSnapshot(const LayerStack& stack) {
    LayerStackSnapshot snapshot;
    snapshot.selectedIndex = stack.GetSelectedIndex();
    
    int count = stack.GetCount();
    snapshot.layers.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        Layer* layer = stack.GetLayer(i);
        LayerSnapshot ls;
        ls.name = layer->name;
        ls.width = layer->width;
        ls.height = layer->height;
        ls.opacity = layer->opacity;
        ls.blendMode = layer->blendMode;
        ls.visible = layer->visible;
        ls.locked = layer->locked;
        
        // Copy CPU pixels cache
        ls.pixels = layer->GetCpuPixels();
        
        snapshot.layers.push_back(std::move(ls));
    }
    
    return snapshot;
}

void HistoryManager::RestoreSnapshot(const LayerStackSnapshot& snapshot, LayerStack& stack) {
    stack.Clear();
    
    for (const auto& ls : snapshot.layers) {
        Layer* newLayer = stack.AddLayer(ls.name, ls.width, ls.height, ls.pixels.data());
        newLayer->opacity = ls.opacity;
        newLayer->blendMode = ls.blendMode;
        newLayer->visible = ls.visible;
        newLayer->locked = ls.locked;
    }
    
    stack.SetSelectedIndex(snapshot.selectedIndex);
}

} // namespace core
