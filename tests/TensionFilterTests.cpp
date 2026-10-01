#include "core/analysis/TensionFilter.h"
#include <cstdlib>
#include <iostream>
using namespace smartimproviser::harmony;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
int main()
{
    std::vector<TensionFilterItem> items = {{0,3,false,false},{1,0,true,false},
        {2,1,false,false},{3,2,false,false},{4,0,false,false},{5,2,false,true}};
    auto all = filteredTensionRows(items,0);
    check(all.size()==5 && all[0].materialIndex==1 && all[0].level==0 && all[0].baseMode, "base first and unclassified");
    check(all[1].materialIndex==2 && all[2].materialIndex==3 && all[3].materialIndex==0 && all[4].materialIndex==4, "all includes ordered labels and unmarked");
    for (int filter : {1,2,3})
    {
        const auto rows = filteredTensionRows(items,filter);
        check(rows.size()==2 && rows[0].materialIndex==1 && rows[1].level==filter, "exact filter plus base");
        check(filteredMaterialSelection(rows,4,filter)==rows[1].materialIndex, "hidden selection replaced by matching material");
        check(filteredMaterialSelection(rows,1,filter)==1, "explicit base selection preserved");
        check(hasTensionRecommendation(rows), "matching recommendation exists");
    }
    items[3].level=0; // Clear the active T2 in another instance.
    const auto empty = filteredTensionRows(items,2);
    check(empty.size()==1 && !hasTensionRecommendation(empty), "unmarked and hidden entries cannot fill empty level");
    check(filteredMaterialSelection(empty,3,2)==1, "removed active label falls back to base");
    check(filteredTensionRows(items,0).size()==5, "All still permits editing unmarked material");
    items[1].hidden=true; // No compatible base in this context.
    const auto none = filteredTensionRows(items,2);
    check(none.empty() && filteredMaterialSelection(none,3,2)==-1, "empty context has no stale invisible selection");
    items[5].hidden=false; // A new compatible catalog entry appears after a chord change.
    const auto next = filteredTensionRows(items,2);
    check(next.size()==1 && filteredMaterialSelection(next,-1,2)==5, "new context selects new matching entry");
    check(normalizedTensionFilter(-1)==0 && normalizedTensionFilter(99)==0, "invalid request normalizes to All");
    std::cout << "Tension filtering checks passed\n";
}
