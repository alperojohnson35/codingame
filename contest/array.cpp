#include <chrono>

using namespace std;
using namespace std::chrono;

#define NOW high_resolution_clock::now()
#define START __time__start = NOW
#define TIME duration_cast<duration<double>>(NOW - __time__start).count()
high_resolution_clock::time_point __time__start = NOW;


template<class T, size_t N>
class Array {
  public:
    size_t fe;
    T data[N];

    Array() {
      fe = 0;
    }

    Array(size_t fe) {
      this->fe = fe;
    }

    inline T& operator[](size_t i) {
      ASSERT(i < N);
      return data[i];
    }

    inline const T& operator[](size_t i) const {
      ASSERT(i < N);
      return data[i];
    }

    inline void operator=(const Array<T, N> &array) {
      fe = array.fe;

      for (size_t i = 0; i < fe; i++) {
        data[i] = array.data[i];
      }
    }

    inline bool full() {
      ASSERT(fe <= N);
      return fe == N;
    }

    inline bool empty() {
      ASSERT(fe <= N);
      ASSERT(fe >= 0);
      return !fe;
    }

    inline bool active() {
      ASSERT(fe <= N);
      ASSERT(fe >= 0);
      return fe;
    }

    inline T& inc() {
      ASSERT(fe < N);
      return data[fe++];
    }

    inline T& last() {
      ASSERT(fe);
      return data[fe - 1];
    }

    inline void dec() {
      ASSERT(fe > 0);
      fe -= 1;
    }

    inline auto begin() {
      return data;
    }

    inline auto end() {
      ASSERT(fe <= N);
      return data + fe;
    }

    inline const auto begin() const {
      return data;
    }

    inline const auto end() const {
      ASSERT(fe <= N);
      return data + fe;
    }

    inline void clear() {
      fe = 0;
    }

    inline void remove(size_t i) {
      ASSERT(fe > 0);
      ASSERT(i < fe);

      dec();
      data[i] = data[fe];
    }
};
