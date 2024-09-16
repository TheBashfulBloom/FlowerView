#include "id_array.h"

// 构造函数，初始化空数组和文件名
MatchIdArray::MatchIdArray()
    : match_id(nullptr), n_match_id(0), max_match_id(0), filename("forbidden_playlist.ini") {}

// 拷贝构造函数
MatchIdArray::MatchIdArray(const MatchIdArray& other)
    : match_id(nullptr), n_match_id(0), max_match_id(0), filename(other.filename) {
    *this = other;  // 使用赋值运算符进行深拷贝
}

// 析构函数，释放内存
MatchIdArray::~MatchIdArray() {
    free(match_id);
}

// 重载赋值运算符，实现深拷贝
MatchIdArray& MatchIdArray::operator=(const MatchIdArray& other) {
    if (this != &other) {  // 避免自赋值
        max_match_id = other.max_match_id > 1?other.max_match_id:1;
        filename = other.filename;  // 复制文件名

        // 尝试重新分配内存
        this->resize(max_match_id);
        //this->n_match_id = 0;
        //qint64* newMatchId = NULL;
        //newMatchId  = static_cast<qint64*>(std::realloc(match_id, max_match_id * sizeof(qint64)));

        // 检查内存分配是否成功
        if (this) {
            n_match_id = other.n_match_id;
            std::memcpy(match_id, other.match_id, n_match_id * sizeof(qint64));
        }
        else{
            throw std::bad_alloc();  // 或者采取其他的错误处理方式
        }
    }
    return *this;
}

// 获取当前有效数组长度
int MatchIdArray::getSize() const {
    return n_match_id;
}

// 获取数组空间的最大长度
int MatchIdArray::getMaxSize() const {
    return max_match_id;
}

// 修改数组空间的最大长度
void MatchIdArray::resize(int newMaxSize) {
    if (newMaxSize < n_match_id) {
        n_match_id = newMaxSize;  // 如果新的最大长度小于当前有效长度，则截断有效长度
    }
    max_match_id = newMaxSize;
    qint64* newMatchId = static_cast<qint64*>(std::realloc(match_id, max_match_id * sizeof(qint64)));

    if (!newMatchId) {
        throw std::bad_alloc();  // 或者采取其他的错误处理方式
    }
    match_id = newMatchId;
}

// 添加一个新的元素到数组中
void MatchIdArray::add(qint64 value) {
    if (!contains(value)) {  // 检查是否包含该元素
        if (n_match_id >= max_match_id) {
            resize((max_match_id == 0) ? 1 : max_match_id * 2);  // 增加容量
        }
        match_id[n_match_id++] = value;
    }
}
// 从数组去掉元素
bool MatchIdArray::remove(qint64 value) {
    // 查找值在数组中的位置
    qint64* found = std::find(match_id, match_id + n_match_id, value);

    // 如果找到了值
    if (found != match_id + n_match_id) {
        // 移动后续元素向前覆盖找到的元素
        std::memmove(found, found + 1, (n_match_id - (found - match_id) - 1) * sizeof(qint64));

        // 减少有效元素数量
        --n_match_id;

        return true;  // 成功移除元素
    }

    return false;  // 未找到元素，无动作
}

    // 获取元素
qint64 MatchIdArray::getMatchId(int index) {
    if(index < this->getSize()){
        return match_id[index];
    }
    else { return -1;
    }
}

// 从大到小排序
void MatchIdArray::sortDescending() {
    std::sort(match_id, match_id + n_match_id, std::greater<qint64>());
}

// 重载 + 运算符，实现数组拼接并去重
MatchIdArray MatchIdArray::operator+(const MatchIdArray& other) const {
    MatchIdArray result;
    result.resize(n_match_id + other.n_match_id);

    // 复制当前数组的元素
    std::memcpy(result.match_id, match_id, n_match_id * sizeof(qint64));
    result.n_match_id = n_match_id;

    // 复制另一数组的元素
    for (int i = 0; i < other.n_match_id; ++i) {
        if (!result.contains(other.match_id[i])) {
            result.add(other.match_id[i]);
        }
    }
    return result;
}

// 重载 - 运算符，实现从一个数组中去掉另一个数组中的成员
MatchIdArray MatchIdArray::operator-(const MatchIdArray& other) const {
    MatchIdArray result;
    result.resize(n_match_id);

    // 仅保留当前数组中不在另一个数组中的元素
    for (int i = 0; i < n_match_id; ++i) {
        if (!other.contains(match_id[i])) {
            result.add(match_id[i]);
        }
    }
    return result;
}

// 检查数组中是否包含某个元素
bool MatchIdArray::contains(qint64 value) const {
    return std::find(match_id, match_id + n_match_id, value) != (match_id + n_match_id);
}

// 去除数组中的重复元素
void MatchIdArray::removeDuplicates() {
    if (n_match_id < 2) return;  // 如果数组长度小于2，则无需处理

    std::sort(match_id, match_id + n_match_id);  // 首先对数组进行排序
    int uniqueIndex = 0;

    // 移动非重复元素到数组前部
    for (int i = 1; i < n_match_id; ++i) {
        if (match_id[i] != match_id[uniqueIndex]) {
            match_id[++uniqueIndex] = match_id[i];
        }
    }

    // 调整有效长度以去除重复元素
    n_match_id = uniqueIndex + 1;
}

// 保存数组到文件
bool MatchIdArray::saveToFile() const {
    QFile file(filename);  // 使用成员变量 filename
    if (!file.open(QIODevice::WriteOnly)) {
        return false;  // 如果文件无法打开，返回 false
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15);  // 设置数据流的版本

    // 写入数组的有效长度和最大容量
    out << n_match_id;
    out << max_match_id;

    // 写入数组内容
    if (match_id && n_match_id > 0) {
        out.writeRawData(reinterpret_cast<const char*>(match_id), n_match_id * sizeof(qint64));
    }

    file.close();
    return true;  // 成功保存文件
}

// 从文件加载数组
bool MatchIdArray::loadFromFile() {
    QFile file(filename);  // 使用成员变量 filename
    if (!file.open(QIODevice::ReadOnly)) {
        return false;  // 文件无法打开
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_15);  // 设置数据流的版本

    // 读取数组的大小和容量
    in >> n_match_id;
    in >> max_match_id;

    // 重新分配内存
    qint64* newMatchId = static_cast<qint64*>(std::realloc(match_id, max_match_id * sizeof(qint64)));

    if (!newMatchId) {
        file.close();
        return false;  // 内存分配失败
    }
    match_id = newMatchId;

    // 读取数组内容
    in.readRawData(reinterpret_cast<char*>(match_id), n_match_id * sizeof(qint64));

    file.close();
    return true;  // 成功读取文件
}

// 根据上下限筛选元素，删除超出范围的元素
bool MatchIdArray::filterRange(qint64 lowerBound, qint64 upperBound) {
    // 确保 lowerBound <= upperBound
    if (lowerBound > upperBound) {
        std::swap(lowerBound, upperBound);
    }

    // 记录旧的有效数据长度
    int oldSize = n_match_id;
    int newSize = 0;

    // 遍历数组，筛选范围内的元素
    for (int i = 0; i < oldSize; ++i) {
        if (match_id[i] >= lowerBound && match_id[i] <= upperBound) {
            match_id[newSize++] = match_id[i];
        }
    }

    // 检查数据是否有变动
    bool hasChanged = (newSize != oldSize);

    // 更新有效数据长度
    n_match_id = newSize;

    return hasChanged;
}



// 更新文件内容：将文件数组中大于当前数组中的最小值的元素截除，然后与当前数组合并，并将结果保存回文件
void MatchIdArray::updateFileWithFilteredArray() {
    MatchIdArray fileArray;
    bool fileLoaded = fileArray.loadFromFile();  // 尝试读取文件

    if (n_match_id == 0) {
        return;  // 当前数组为空，不需要进行任何操作
    }

    qint64 minCurrentValue = *std::min_element(match_id, match_id + n_match_id);

    MatchIdArray filteredArray;
    filteredArray.resize(fileArray.n_match_id);

    if (fileLoaded) {  // 如果文件成功读取，进行过滤操作
        for (int i = 0; i < fileArray.n_match_id; ++i) {
            if (fileArray.match_id[i] < minCurrentValue) {
                filteredArray.add(fileArray.match_id[i]);
            }
        }
    }

    // 如果文件未能加载，filteredArray 将保持为空
    MatchIdArray combinedArray = *this + filteredArray;  // 将当前数组与过滤后的文件数组合并

    combinedArray.saveToFile();  // 保存合并后的数组到文件
}

