#include "Transform.h"

namespace engine {
namespace math {

    inline Transform& Transform::operator=(const Transform& i_transform)
    {
        // check for self assignment
        if (this != &i_transform)
        {
            position_ = i_transform.position_;
            rotation_ = i_transform.rotation_;
            scale_ = i_transform.scale_;
        }
        return *this;
    }

} // namespace math
} // namespace engine