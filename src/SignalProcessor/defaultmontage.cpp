#include "defaultmontage.h"

#include "../../Alenka-File/include/AlenkaFile/datamodel.h"

#include <algorithm>

using namespace std;
using namespace AlenkaFile;

namespace {

/**
 * @brief Returns whether the labels are unique.
 */
bool labelsUnique(const AbstractTrackTable *table) {
  const int count = table->rowCount();
  vector<string> labels;
  labels.reserve(count);

  for (int i = 0; i < count; ++i)
    labels.push_back(table->row(i).label);

  sort(labels.begin(), labels.end());
  auto end = unique(labels.begin(), labels.end());
  return end == labels.end(); // This is true if no elements were erased.
}

} // namespace

void DefaultMontage::fillTrackTable(const AbstractTrackTable *recordingMontage,
                                    const AbstractTrackTable * /*output*/,
                                    int outputIndex,
                                    UndoCommandFactory *undoFactory) {
  const int count = recordingMontage->rowCount();
  undoFactory->insertTrack(outputIndex, 0, count);

  // Fill defaultTracks with suitable code. Using the labels makes sense only
  // when they can be reliably used to identify the channels -- i.e. when they
  // are unique.
  if (labelsUnique(recordingMontage)) {
    for (int i = 0; i < count; ++i) {
      Track t = recordingMontage->row(i);
      t.code = "out = in(\"" + t.label + "\");";
      undoFactory->changeTrack(outputIndex, i, t);
    }
  } else {
    for (int i = 0; i < count; ++i) {
      Track t = recordingMontage->row(i);
      t.code = "out = in(" + to_string(i) + ");";
      undoFactory->changeTrack(outputIndex, i, t);
    }
  }
}
