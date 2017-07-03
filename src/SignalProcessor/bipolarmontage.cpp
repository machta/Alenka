#include "bipolarmontage.h"

#include "../../Alenka-File/include/AlenkaFile/datamodel.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace AlenkaFile;

namespace {

void parseLabels(const AbstractTrackTable *source, vector<string> &prefixes,
                 vector<int> &indexes) {
  int sourceRows = source->rowCount();

  for (int i = 0; i < sourceRows; ++i) {
    Track t = source->row(i);
    string label = t.label;

    auto prefixEnd = find_if(label.begin(), label.end(),
                             [](char c) { return isalpha(c) == 0; });
    string prefix(label.begin(), prefixEnd);

    auto indexEnd =
        find_if(prefixEnd, label.end(), [](char c) { return isdigit(c) == 0; });
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

} // nemespace

void BipolarMontage::fillTrackTable(const AbstractTrackTable *source,
                                    AbstractTrackTable *output) {
  vector<string> prefixes;
  vector<int> indexes;
  int outputSize = output->rowCount();

  parseLabels(source, prefixes, indexes);
  assert(prefixes.size() == indexes.size());
  assert(static_cast<int>(prefixes.size()) == source->rowCount());

  for (unsigned int i = 0; i < prefixes.size(); ++i) {
    int match = matchPair(i, prefixes, indexes);

    if (0 <= match) {
      output->insertRows(outputSize);

      Track t = output->row(outputSize);

      t.label =
          prefixes[i] + to_string(indexes[i]) + "-" + to_string(indexes[match]);

      string lA = prefixes[i] + to_string(indexes[i]);
      string lB = prefixes[i] + to_string(indexes[match]);
      t.code = "out = in(\"" + lA + "\") - in(\"" + lB + "\");";

      output->row(outputSize++, t);
    }
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
