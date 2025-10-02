# 台灯模型注意事项

## 1. 在OpenGL中将贴图贴到柱状物体上

使用shader: cyl.vs, cyl.fs实现，实现较为简单粗略，可能出现问题，仅供参考，期待你的优化改进

## 2. 给同一个model中的不同mesh使用不同的shader

可自定义修改 mesh.h, model.h 等文件，但是报告中要说明清楚

也可以使用下面的“办法”，较为简略，仅供参考，相信你能给出更好的方法

```c
...
for (unsigned int i = 0; i < lamp.meshes.size(); i++)
{
    auto& m = lamp.meshes[i];
    for (auto& t : m.textures) 
    {
        if (t.path == "wood1.jpg") {
            basicShader.use();
            basicShader.setMat4("projection", projection);
            basicShader.setMat4("view", view);
            basicShader.setMat4("model", model);
            m.Draw(basicShader);
        }
        else if (t.path == "painting1.jpg") {
            cylShader.use();
            cylShader.setMat4("projection", projection);
            cylShader.setMat4("view", view);
            cylShader.setMat4("model", model);

            float yMin = +1e9, yMax = -1e9;
            for (auto& m : lamp.meshes) {
                for (auto& v : m.vertices) {
                    if (v.Position.y < yMin) yMin = v.Position.y;
                    if (v.Position.y > yMax) yMax = v.Position.y;
                }
            }
            cylShader.setFloat("yMin", yMin);
            cylShader.setFloat("yMax", yMax);
            m.Draw(cylShader);
        }
    }
}
...
```
