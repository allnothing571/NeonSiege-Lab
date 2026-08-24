#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Entity {
public:
    Entity(const std::string& name, int hp)
        // TODO 1：使用初始化列表初始化 name_ 和 hp_。
    {
    }

    void takeDamage(int damage) {
        // TODO 2：忽略非正数伤害，并保证 hp_ 不小于 0。
    }

    bool isAlive() const {
        // TODO 3：返回实体是否存活。
        return false;
    }

    const std::string& name() const {
        return name_;
    }

    int hp() const {
        return hp_;
    }

private:
    std::string name_;
    int hp_ = 0;
};

int main() {
    std::vector<Entity> entities;

    // TODO 4：创建至少 3 个实体并加入 entities。
    // TODO 5：使用引用遍历，让其中至少 1 个实体死亡。
    // TODO 6：使用 erase-remove_if 删除死亡实体。

    std::ofstream output("day1_entities.txt");
    if (!output) {
        std::cerr << "无法创建 day1_entities.txt\n";
        return 1;
    }

    // TODO 7：把存活实体写入文件，并同步输出到控制台。

    return 0;
}

