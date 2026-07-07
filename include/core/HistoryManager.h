#pragma once
#include <string>
#include <vector>
#include "core/Layer.h"
#include "core/LayerStack.h"

namespace core {

struct LayerSnapshot {
    std::string name;
    std::vector<unsigned char> pixels;
    int width;
    int height;
    float opacity;
    BlendMode blendMode;
    bool visible;
    bool locked;
};

struct LayerStackSnapshot {
    std::vector<LayerSnapshot> layers;
    int selectedIndex;
};

class HistoryManager {
public:
    HistoryManager(int maxStates = 50);
    ~HistoryManager();

    // Record the current state of a LayerStack
    void RecordState(const LayerStack& stack, const std::string& actionName);

    // Perform Undo / Redo
    bool Undo(LayerStack& stack);
    bool Redo(LayerStack& stack);

    // Get lists for UI panel
    const std::vector<std::string>& GetActionHistory() const { return m_ActionNames; }
    int GetCurrentStateIndex() const { return m_CurrentIndex; }
    
    // Jump to specific state in history
    bool JumpToState(int index, LayerStack& stack);

    // Clear history
    void Clear();

    bool CanUndo() const { return m_CurrentIndex > 0; }
    bool CanRedo() const { return m_CurrentIndex < static_cast<int>(m_History.size()) - 1; }

private:
    void RestoreSnapshot(const LayerStackSnapshot& snapshot, LayerStack& stack);
    LayerStackSnapshot CreateSnapshot(const LayerStack& stack);

    int m_MaxStates;
    std::vector<LayerStackSnapshot> m_History;
    std::vector<std::string> m_ActionNames;
    int m_CurrentIndex;
};

} // namespace core
