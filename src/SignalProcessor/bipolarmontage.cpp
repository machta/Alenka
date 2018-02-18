#include "bipolarmontage.h"

#include "../../Alenka-File/include/AlenkaFile/datamodel.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace AlenkaFile;

namespace {

void parseLabels(const AbstractTrackTable *recordingMontage,
                 vector<string> &prefixes, vector<int> &indexes) {
  int recordingMontageRows = recordingMontage->rowCount();

  for (int i = 0; i < recordingMontageRows; ++i) {
    Track t = recordingMontage->row(i);
    string label = t.label;
    auto digit = [](char c) { return isdigit(c) == 0; };

    auto prefixEnd = find_if_not(label.begin(), label.end(), digit);
    string prefix(label.begin(), prefixEnd);

    auto indexEnd = find_if(prefixEnd, label.end(), digit);
    string index(prefixEnd, indexEnd);

    if (indexEnd != label.end() || prefix.empty() || index.empty()) {
      prefixes.push_back("");
      indexes.push_back(-1);
    } else {
      prefixes.push_back(prefix);
      indexes.push_back(stoi(index));
    }
  }
}

} // namespace

void BipolarMontage::fillTrackTable(const AbstractTrackTable *recordingMontage,
                                    const AbstractTrackTable *output,
                                    int outputIndex,
                                    UndoCommandFactory *undoFactory) {
  vector<string> prefixes;
  vector<int> indexes;
  int trackIndex = 0;

  parseLabels(recordingMontage, prefixes, indexes);
  assert(prefixes.size() == indexes.size());
  assert(static_cast<int>(prefixes.size()) == recordingMontage->rowCount());

  vector<string> labels, codes;

  for (unsigned int i = 0; i < prefixes.size(); ++i) {
    int match = matchPair(i, prefixes, indexes);

    if (0 <= match) {
      labels.push_back(prefixes[i] + to_string(indexes[i]) + "-" +
                       to_string(indexes[match]));

      string lA = prefixes[i] + to_string(indexes[i]);
      string lB = prefixes[i] + to_string(indexes[match]);
      codes.push_back("out = in(\"" + lA + "\") - in(\"" + lB + "\");");
    }
  }

  int count = labels.size();
  assert(count == static_cast<int>(codes.size()));
  undoFactory->insertTrack(outputIndex, 0, count);

  for (int i = 0; i < count; ++i) {
    Track t = output->row(trackIndex);
    t.label = labels[i];
    t.code = codes[i];
    undoFactory->changeTrack(outputIndex, i, t);
  }
}

int BipolarMontage::matchPair(int i, const vector<string> &prefixes,
                              const vector<int> &indexes) {
  string prefix = prefixes[i];
  int index = indexes[i], minMatch = -1;

  for (unsigned int j = 0; j < prefixes.size(); ++j) {
    if (prefix != prefixes[j])
      continue;

    if (index < indexes[j] &&
        (minMatch == -1 || indexes[j] < indexes[minMatch]))
      minMatch = j;
  }

  return minMatch;
}

int BipolarNeighboursMontage::matchPair(int i, const vector<string> &prefixes,
                                        const vector<int> &indexes) {
  int match = BipolarMontage::matchPair(i, prefixes, indexes);

  if (0 <= match && indexes[i] + 1 != indexes[match])
    match = -1;

  return match;
}
