
typedef int SortComparator(const void *a, const void *b, void *user_data);
Function void Sort (void *base, size_t n_items, size_t bytes_per_item, SortComparator *comparator, void *user_data);
