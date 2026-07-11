#include "core/DocumentCommands.h"
#include <cstdio>

namespace core::commands {

void NewLayer(LayerStack& stack, HistoryManager& history, int imgWidth, int imgHeight) {
    char name[64];
    snprintf(name, sizeof(name), "Layer %d", stack.GetCount() + 1);
    history.RecordState(stack, "New Layer");
    stack.AddLayer(name, imgWidth, imgHeight, nullptr);
}

void DuplicateLayer(LayerStack& stack, HistoryManager& history, int selectedIdx) {
    if (selectedIdx == -1) return;
    history.RecordState(stack, "Duplicate Layer");
    stack.DuplicateLayer(selectedIdx);
}

void DeleteLayer(LayerStack& stack, HistoryManager& history, int selectedIdx) {
    if (selectedIdx == -1 || stack.GetCount() <= 1) return;
    history.RecordState(stack, "Delete Layer");
    stack.DeleteLayer(selectedIdx);
}

void MergeDown(LayerStack& stack, HistoryManager& history, int selectedIdx) {
    if (selectedIdx <= 0) return;
    history.RecordState(stack, "Merge Down");
    stack.MergeDown(selectedIdx);
}

void MergeVisible(LayerStack& stack, HistoryManager& history, int width, int height) {
    if (stack.GetCount() <= 1) return;
    history.RecordState(stack, "Merge Visible");
    stack.MergeVisible(width, height);
}

void MoveLayerUp(LayerStack& stack, int selectedIdx) {
    if (selectedIdx < stack.GetCount() - 1) {
        stack.MoveLayer(selectedIdx, selectedIdx + 1);
    }
}

void MoveLayerDown(LayerStack& stack, int selectedIdx) {
    if (selectedIdx > 0) {
        stack.MoveLayer(selectedIdx, selectedIdx - 1);
    }
}

void SelectLayerUp(LayerStack& stack, int selectedIdx) {
    if (selectedIdx < stack.GetCount() - 1) {
        stack.SetSelectedIndex(selectedIdx + 1);
    }
}

void SelectLayerDown(LayerStack& stack, int selectedIdx) {
    if (selectedIdx > 0) {
        stack.SetSelectedIndex(selectedIdx - 1);
    }
}

} // namespace core::commands
