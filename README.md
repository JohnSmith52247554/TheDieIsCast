# The Die Is Cast

**AveMujica**

> 本项目是2025年清华大学电子工程系“拓竹杯”软件设计大赛参赛作品，编号66
## 设计说明

《The Die is Cast》是AveMujica制作组制作的一款以时间倒流为特色的2D横板RPG游戏。本游戏使用C++和SFML（一款基于OpenGL等API的多媒体库）开发，未使用任何游戏引擎。

本游戏从主角Kyrion与女主Mnema梦中的相遇起笔，从主角对梦境真相的探寻中，逐渐发现时间倒流与创造时间倒流装置的组织的真相，以及揭开女主Mnema身份真相之谜。本游戏玩法新颖，在一般RPG操作之外加入了控制游戏内时间倒流操作，允许玩家在不断回溯中拥有无限反悔机会，同时发掘时间倒流背后潜藏的秘密；此外，游戏拥有精巧设计的解谜关卡，为玩家带来异彩纷呈的烧脑盛宴。

## 使用说明

由于中文输入法往往会拦截键盘事件，请在游玩时**切换至英文输入法**。

| **功能** | **键位**                                        |
| ------ | --------------------------------------------- |
| 移动     | AD或左右方向键                                      |
| 跳跃     | 空格键或W或上方向键                                    |
| 逆转时间   | 左右shift                                       |
| 对话     | 按下空格键或回车键进入下一句对话，使用上下方向键选择选项，并使用回车键或空格键确认选项   |
| 对话快进   | 左右ctrl                                        |
| 交互     | 靠近可交互的实体并按下F                                  |
| 呼出暂停界面 | esc                                           |
| 切换全屏   | F10                                           |
| 启动作弊模式 | 左右ctrl。启用作弊模式需要在源码中将SHOW_CONSOLE设置为true，并重新编译 |

## 项目特色

1. **自主开发**：未使用任何游戏引擎，从零开始进行开发，目前已自主编写了一万两千行代码
2. **跨平台**：得益于开发工具较为底层，本游戏具有极强的跨平台兼容性，目前已推出windows版，macOS版（Intel & AppleSilicon）以及Linux版
3. **低耦合，易扩展**：使用**消息队列**实现模块间的通讯，降低耦合度；脚本和地图数据储存于JSON文件和TMX文件中，易于修改和扩展
4. **低资源占用**：内存占用通常不超过50mb，CPU占用更是微乎其微
5. **分支剧情**：您的选择将决定故事的结局
6. **高鲁棒性**：无内存泄漏问题；具有异常处理机制，可以处理资源文件缺失的情况

## 平台

- windows
- macOS
- Linux

windows最低支持windows 10 2004(2020年5月更新)
macOS最低支持macOS 11.3
Linux目前尚未测试最低兼容的系统版本，不过只要不是太老的系统应该都没问题

## 核心技术

1. 使用**消息队列**实现游戏实体之间的通讯，为了配套消息系统，设计了可自动分配的**身份编码**
2. 使用**快照机制**保存游戏状态，以实现时间倒流的效果；定期将快照**异步**写入磁盘，避免内存占用过高；不同游戏实体之间需要记录的信息各不相同，为降低耦合性，设计了16位**特征编码**来记录差异化的信息
3. 使用统一的**资源管理器**来管理贴图、音乐、音效等资源，避免资源拷贝，降低内存占用
4. 使用**Shader**实现时间倒流的特效

## 文件结构

	TheDieIsCast/         # 源代码、脚本及资源文件
	|--External/
	|   |--include/       # 附加库头文件
	|   |--Frameworks_64/ #macOS动态链接所需
	|   |--Frameworks_arm/
	|   |--lib_win32/     # 适用于不同系统的SFML库文件
	|   |--lib_mac_64/ 
	|   |--lib_mac_arm/
	|   └--lib_linux/
	|--History/
	|   |--save.bin       # 用户存档
	|   └--timeline.bin   # 用于实现时间逆转的记录文件
	|--Include/           # 头文件
	|--Resources/         # 资源文件
	|   |--Background/    # 场景背景
	|   |--CG/
	|   |--Fonts/
	|   |--Icon/
	|   |--Images/        # 实体贴图和动画
	|   |--Maps/
	|   |--Music/         # 背景音乐
	|   |--Shader/        # 着色器
	|   |--Sounds/        # 音效
	|   └--Info.plist     # 用于在macOS中创建应用 
	|--Script/            # 脚本
	|--Source/            # 源文件
	|--*.dll
	|--CMakeLists.txt
	|--EncodingConverter.py   # 赠品，用于批量转换编码
	|--README.md
	└--LICENSE.md

## 构建与编译

请使用**cmake**（3.12及以上）进行构建

SFML在不同平台下需使用不同的编译器进行编译，请按照下表所示选择编译器

| 系统        | 编译器                                                                           |
| --------- | ----------------------------------------------------------------------------- |
| windows   | visual c++ 17 32bit (visual studio 2022)<br>gcc (需要在CMakeLists.txt中修改连接的库的版本) |
| macOS x64 | clang 64bit                                                                   |
| macOS arm | clang arm                                                                     |
| Linux     | gcc                                                                           |

如果要在windows中使用gcc编译，则需要修改CMakeLists.txt以连接到gcc版本的库文件

```cmake
// line 36
// 将这一行
set(SFML_ROOT ${CMAKE_SOURCE_DIR}/External/lib_win32)
// 改成
set(SFML_ROOT ${CMAKE_SOURCE_DIR}/External/lib_win32_gcc)
```

若使用MSVC，请点击build文件夹中生成的.sln，并使用visual studio编译；
若使用其他编译器，cmake将生成makefile，请在终端运行以下命令以编译

```bash
cd build
make
```

由于不同编译器对于中文字符串的支持程度不同，编译前可能需要转换文件编码
所有文件均使用**GB 2312**编码，若是MSVC编译，则不需要转换编码；若使用clang或gcc，则需要将Interface.cpp和Page.cpp（包含中文字符串）的编码转化为**utf-8**
可以使用附赠的编码转换工具进行批量处理

```bash
python EncodingConverter.py Source -e utf-8
```

编译完成后，请务必将输出的可执行文件复制至**CMakeLists所在的目录**，以确保游戏可以读取资源文件
编译后的程序只需要External/，Resources/，Script/和History/中的资源即可运行（windows版本还需dll），其他目录均可删除

亦可[点此](https://github.com/SFML/SFML/releases/tag/2.6.2)下载其他版本的的SFML2.6.2库文件，并自行配置CMakeLists以构建项目

## 剧情分支

**以下内容涉及严重剧透，请尽量仅在卡关时阅读**

【分支1】
<地点：神秘组织内部 [开发组编号：地图六]>
【选择1：加入】
{
走向【NORMAL ENDING 1】
{
【选择2：不加入】或【完成选择1后从神秘组织内部开始直接开启时间倒流】
{
【剧情继续进行】
}

【分支2】
<地点：主角持续时间逆转直至回到捡起头套一刻 [开发组编号：地图二]>
/*自动判定，无需选择*/
【情况1：未收集到【时序能源】道具】
{
走向【BAD ENDING】
}
【情况2：收集到【时序能源】道具】
{
【剧情继续进行】
}
/*【时序能源】为隐藏在主线进入神秘组织大门后第三个关卡的夹层中的特殊道具*/

【分支3】
<地点：主角博士期间的实验室 [开发组编号：地图十]>
【情况1：未达成过【NORMAL ENDING 1】且未阅读过主线关卡中隐藏的两个日志】
{
走向【NORMAL ENDING 2】
}
【情况2：达成过【NORMAL ENDING 1】或阅读过主线关卡中隐藏的两个日志】
{
【剧情继续进行】

【分支4】
<地点：主角尝试劝阻Mnema父母的游艇上 [开发组编号：梦世界2]>
【情况1：选择Mnema】
{
走向【ENDING 1】
}
【情况2：选择阻止世界大战】
{
走向【ENDING 2】
}

## github repository

https://github.com/JohnSmith52247554/TheDieIsCast

## 贡献

| 姓名  | 分工      | 联系方式                          |
| --- | ------- | ----------------------------- |
| 胡云淏 | 程序      | hu-yh24@mails.tsinghua.edu.cn |
| 姜胜凯 | 关卡设计、程序 | jsk24@mails.tsinghua.edu.cn   |
| 雷昱  | 文案、音乐   | gentyakylone@gmail.com        |
| 王岩  | 美术      | wy13965799742@qq.com          |
| 温旭  | 美术      | 1972755916@qq.com             |

## 开源协议

本项目使用MIT开源协议