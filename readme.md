

**UE VERSION:** 5.6

# 项目与引擎配置

## 资源管理类UAuraAssetManager

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1722701474889-7b57f28b-3c10-4af3-bc6f-c1026fec9807.png)

<font style="background-color:#E7E9E8;">UAuraAssetManager</font> 继承 `UAssetManager` 在初始化引擎加载时 调用 <font style="background-color:#E7E9E8;">FAuraGmaeplayTags::InitNaviveGameplayTags</font> 添加标签并且为 单例结构体 <font style="background-color:#E7E9E8;">FAuraGmaeplayTags</font> 的成员赋值，为了方便直接通过 <font style="background-color:#E7E9E8;">FAuraGmaeplayTags</font>  设置 Tag 和使用 Tag

### 为了正常使用ASC的TargetData同步功能，需要在自定义AssetManager中

```c++
void UAuraAssetManager::StartInitialLoading()
{
Super::StartInitialLoading();
FAuraGameplayTags::InitializeNativeGameplayTags();
UAbilitySystemGlobals::Get().InitGlobalData();
}
```

## 渲染与效果

卡通画风，项目素材却是4096级别，可以统一限制在512级别，

修改纹理流送池

```ini
[/Script/Engine.RenderSettings]
r.TextureStreaming=True
r.Streaming.PoolSize=1000
```

## 

# Actor

## AAuraEffectActor

进行某些GAS相关的触发

定义的其他辅助类型

+ <font style="background-color:#E7E9E8;">EEffectApplicationPolicy</font> <font style="background-color:#E7E9E8;">EEffectRemovalPolicy</font> GE的应用时机和移除时机
+ <font style="background-color:#E7E9E8;">FEffectWithPolicy</font>  GE的结构体，存在GE的类，GE的应用和移除时机，是否销毁Actor

组件

+ 静态网格体（根）
+ 碰撞（用做重叠检测）
+ <font style="background-color:#E7E9E8;">FEffectWithPolicy</font> 结构体数组

# Character类

![画板](https://cdn.nlark.com/yuque/0/2024/jpeg/36214189/1721324949717-935992ed-8cf4-43bc-8a19-3fce385f55c4.jpeg)



项目所有Character都继承**AAuraCharacterBase**,并由其分出敌人和玩家类



函数：

+ <font style="background-color:#E7E9E8;">InitializePrimaryAttribute</font> 通过GE初始化部分属性，在 <font style="background-color:#E7E9E8;">InitAbilityActorInfo</font> 中调用

## AAuraEnemy

不同哥布林建立了不同的蓝图类（枪哥布林，弹弓哥布林...），而不是同一蓝图类因为武器不同而状态不同



因为所有敌人动画逻辑一样，资源不一样，因此建立了动画模板**ABP_Enemy**，子类不同的敌人类型动画蓝图只需对其继承，更改不同的动画资源即可

### 接口IEnemyInterface

游戏是俯视角游戏，不少操作需要鼠标指针交互，因此需要设计鼠标悬浮的响应

鼠标悬浮至不同的对象，需要其做出不同的反应，因此设计一个接口，响应被鼠标悬浮的事件

+ 高亮和取消高亮

### 接口 ICombatInterface

通过这个接口处理一些战斗的接口交互

+ <font style="background-color:#EFF0F0;">GetPlayerLevel</font> 获取实例的等级
+ <font style="background-color:#FBF5CB;">GetCombatSocktLocation</font> 获取插槽位置，用于定位生成的技能
+ <font style="background-color:#FBF5CB;">SetFaceingTargetLoc</font> 用 BlueprintImplementableEvent 宏修饰，旨在蓝图中覆盖，实现设置运动扭曲的相关配置

## AuraCharacter

因为俯视角游戏，镜头默认不旋转，因此不能让角色旋转继承控制器的旋转了，而且摄像机也一样，禁用控制器的旋转

函数

+ <font style="background-color:#EFF0F0;">InitAbilityActorInfo</font> 初始化GAS的 Onwer 和 Avatar, 初始化UI的控制层

### PlayerState

切换角色，角色重生**<font style="color:#DF2A3F;">不希望重写构造设置一些内容</font>**，因此将角色的部分内容放置到该类中

保存玩家状态，包括将Gas挂载到这个下面

成员变量

+ 等级 Level : 没必要放入Attribute，非浮点，不需要和GAS其他强互动，但是部分属性依赖此进行计算

### 遮挡相机的Actor进行透点蒙版透明化

设置下列值，连接Mask即可

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1726424519159-967285b6-936c-4eb8-9c82-8379ef0dcc4f.png)

直接而简单的添加碰撞盒，如果存在重叠，则通知对象调整材质节点的值进行透点化

![遮挡碰撞盒](https://cdn.nlark.com/yuque/0/2024/png/36214189/1726424577164-900beb91-daf9-4a7b-af19-c0d331b2eba1.png)

> Fade In and Fade Out

1. 创建一个Fade Actor基类，在Construct中保存原始Material数组，遍历Fade数组创建动态材质数组。
2. 自定义两个蓝图事件，在Fade Out中，先遍历动态材质数组，设置 Mesh中的材质为动态材质实例，并根据Timeline设置动态材质的参数。在Fade in中共类似。在TimeLine完成后如果是Fade In则需要恢复Mesh的原始材质。
3. 选择合适的时机调用Fade Out或者Fade In。在角色在Actor后面的时候Fade Out
4. 创建一个蓝图接口，FadeActor实现这个接口，重写FadeOut和FadeIn函数。在角色蓝图中。
5. 在角色相机下面增加一个Box，为该Box设置重叠事件，开始重叠的时候Fade Out，结束重叠的时候

## 角色类型与配置Character Classes

面对不同的角色类型，要怎样进行配置与初始化?

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1723740767688-7550a0ed-5d8a-4b9c-a420-ecebdebf6ad2.png)

+ 创建 `UCharacterClassInfo` 数据资产。

+ 创建 `ECharacterClass` 枚举。

+ 为**属性**创建曲线表。

      - a. `CT_WarriorPrimaryAttributes`（战士主要属性）
      
      - b. `CT_RangerPrimaryAttributes`（游侠主要属性）
      
      - c. `CT_ElementalistPrimaryAttributes`（元素师主要属性）

+ 为主要属性、次要属性和关键属性创建 GE（Gameplay Effect，游戏效果）

      - a. 主要属性不同 次要属性和关键属性靠同一GE计算

+ 有共享技能和效果。

+ 使用数据资产初始化属性的函数。

> 创建DataAsset

1. 在CharacterClassInfo类中创建一个枚举标识不同的职业
2. 创建一个结构体FCharacterClassDefaultInfo，表示不同的职业的不同primaryAttribute GE
3. 在CharacterClassInfo类中保存一个Map，key为枚举，value为FCharacterClassDefaultInfo
4. 早CharacterClassInfo类中存储通用的secondary和baseAttribute初始GE。并提供一个根据枚举获取不同职业的PrimaryAttribute初始化GE
5. 在编辑器中创建这些GE，并在DataAsset蓝图中配置这些GE

> 填写Curve Tables

- 直接填写，选择插值
- CSV/json格式,修改，导入

> 根据DataAsset初始化敌人信息

1. 把DA存放在GameMode中。
2. 在蓝图可调用函数库中设置一个初始化属性得函数，该函数根据传入的CharacterClass，Level和ASC初始化角色属性。
3. 在敌人基类中新增一个类别属性，将角色基类的初始化默认属性函数`InitializeDefaultAttributes`设置为虚函数，主角直接用角色基类的实现，敌人基类重写该实现。在重写过程中调用蓝图可调用函数库中的函数，传入敌人的类别，等级和敌人的ASC。`UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);`

> 遇到错误客户端AuraGameMode空指针

原因：GameMode仅在服务器存在

解决：调用前判断服务器

```c++
if (HasAuthority())
{
    InitializeDefaultAttributes();
}
```

# Player

操控**AuraCharacter**的子蓝图类

## 控制器

建立**<font style="color:#DF2A3F;background-color:#E7E9E8;">AAuraPlayerController</font>**类作为玩家的控制器类该类主要进行：

- 设置增强输入系统，添加增强输入操作

- 默认显示光标

- 设置输入模式，使得鼠标不被锁定到窗口，窗口不隐藏鼠标

- 添加移动输入，由于控制器旋转跟随视角，且项目为俯视角，因此需要获取控制器的Yaw旋转，并且以此获取向前和向右向量来添加移动输入

- 重写**仅有玩家输入才tikc**的事件，在里面进行调用检测鼠标悬浮的对象，以达成鼠标悬浮交互的功能

- 使用 资产管理(DataAsset) 进行**数据驱动输入操作**

- 存在 资产数据 <font style="background-color:#EFF0F0;">InputConfig</font> 选择相应的输入操作资产
  
- 存在 <font style="background-color:#FBF5CB;">按住 按下 松开</font> 三种操作的回调函数，绑定到 <font style="background-color:#EFF0F0;">增强输入操作系统(UAuraInputComponent)</font> 
  
- 该绑定将 tag 和 回调函数进行了绑定，输入操作之后调用回调时，会把tag进行参数传入

### 增强输入操作组件 AuraInputComponent

![设置增强输入类型](https://cdn.nlark.com/yuque/0/2024/png/36214189/1722964429074-edc321c3-de41-4c86-908a-232f8f0b45cf.png)主要是 模板函数 <font style="background-color:#FBF5CB;">BindAbilityActions</font> 

+ 接受不同的函数类型，来<font style="color:#DF2A3F;">进行输入操作和回调函数的绑定</font> ;
+ 接受 资产数据 <font style="background-color:#EFF0F0;">InputConfig</font> 将资产数据内的**<font style="color:#DF2A3F;">标签</font>****作为参数绑定到回调函数**

# GAS

![GAS挂载对象](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721468623784-ef3919bf-4ac4-4bce-8ebd-c298ea25b105.png)

敌人类直接装载GAS相关组件

但是玩家有其他需求，比如重生，切换，此时**<font style="color:#DF2A3F;">不希望GAS相关的组件被销毁之后由新的实例创建，而是希望保持</font>**，因此采用<font style="color:#DF2A3F;">挂载到</font>**<font style="color:#DF2A3F;">playerState</font>**<font style="color:#DF2A3F;">的方式</font>

因此，会存在**<font style="color:#DF2A3F;background-color:#E7E9E8;">OwnerActor</font>**和**<font style="color:#DF2A3F;background-color:#E7E9E8;">AvatarActor</font>**的区别，设置时需要区别

![GAS获取所有者](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721490576558-f8351f1e-200b-4f72-bdc7-680626f3d21d.png)

如上图，因为玩家类的GAS挂载到PlayerState上，因此两个Actor的值不同。



那么，什么时候设置组件的拥有者呢？AI那边自然是简单，因为GAS就在其Character上，因此BeginPlay时，设置必定有效。

但是，玩家这边GAS的**OwnerActor**是**PlayerState**，**AvatarActor**是Character，而且还有网络复制，同步模式为<font style="color:rgb(31, 35, 40);">Mixed</font><font style="color:rgb(31, 35, 40);">（详情见该模式注意事项）</font>

<font style="color:rgb(31, 35, 40);">首先，要清楚，</font>**<font style="color:#DF2A3F;">客户端的实例和服务器上的实例不会完全同时一样</font>**<font style="color:rgb(31, 35, 40);">，因此在客户端上调用对象返回的内容和服务器上调用对象的返回</font>**<font style="color:rgb(31, 35, 40);">可能存在不一样的情况</font>**<font style="color:rgb(31, 35, 40);">，因此 </font>**<font style="color:#DF2A3F;background-color:#EFF0F0;">复制</font>**<font style="color:rgb(31, 35, 40);"> 这个词用的很好 </font>**<font style="color:#DF2A3F;">表示两个实例</font>**<font style="color:rgb(31, 35, 40);">，所以设置GAS的拥有者存在服务端和客户端不同时机设置的情况</font>

**<font style="color:#DF2A3F;">在pawn被控制时，这是服务端干的事，</font>**此时是**<font style="color:#DF2A3F;">服务端</font>****的GAS实例设置拥有者**

**<font style="color:#DF2A3F;">在服务器发出有PlayerState的回调时 </font>**，此时是**<font style="color:#DF2A3F;">客户端</font>****的GAS实例进行设置拥有者**

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721492214252-85062cf7-6a0f-4653-8360-8d550d04bf85.png)

## UAuraAbilitySystemComponent

## AuraGameplayAbility

### `GameplayAbility`

1. 定义技能和能力
2. 能力必须先授予ASC,
   1. 通常在服务器端完成授予。
   2. 产生一个`AbilitySpec`同步到客户端
3. 必须被激活`Actived`才能使用
4. 内置`cost`和`cooldown`
5. 支持异步执行
   1. 同时可以有多个副本
6. Ability Tasks

#### 为`character`添加能力

- `AuraCharacterBase.h`
  - 在`AuraCharacterBase`储存能力数组`TArray<TSubclassOf<UGameplayAbility>>`，
  - 使用`AddCharacterAbilities()`调用`ASC`的`AddCharacterAbilities`添加能力,只在服务器端添加能力
- `AuraAbilitySystemComponent.h`
  - `void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);`
  - 遍历`TArray`创建每一个`FGameplayAbilitySpec`
  - `GiveAbility(FGameplayAbilitySpec)` 或 `GiveAbilityAndActiveOnce(FGameplayAbilitySpec)`(立即激活)赋予能力
- `AuraCharacter.h`
  - 通过`PossessedBy()`函数在服务器端调用`AddCharacterAbilities()`

<font style="background-color:#E7E9E8;">AuraCharacterBase</font> 存在 <font style="background-color:#E7E9E8;">StartupAbilities</font> 用来储存 角色初始拥有的技能

#### GameplayAbility Tag

| Gameplay Tag (游戏性标签)                                 | Description (描述)                                           |
| :-------------------------------------------------------- | :----------------------------------------------------------- |
| **Ability Tags**<br>(能力标签)                            | 该能力拥有这些标签。                                         |
| **Cancel Abilities with Tag**<br>(取消带有特定标签的能力) | 当执行此能力时，带有这些标签的能力会被取消。                 |
| **Block Abilities with Tag**<br>(阻塞带有特定标签的能力)  | 当此能力处于激活状态时，带有这些标签的能力会被阻塞（无法使用）。 |
| **Activation Owned Tags**<br>(激活时拥有的标签)           | 当此能力激活时，应用于激活所有者（Owner）的标签。如果在 `AbilitySystemGlobals` 中启用了 `ReplicateActivationOwnedTags`，这些标签会被网络复制（同步）。 |
| **Activation Required Tags**<br>(激活所需标签)            | 只有当激活该能力的 Actor/组件拥有**所有**这些标签时，此能力才能被激活。 |
| **Activation Blocked Tags**<br>(激活阻塞标签)             | 如果激活该能力的 Actor/组件拥有这些标签中的**任何一个**，此能力将被阻塞（无法激活）。 |
| **Source Required Tags**<br>(源所需标签)                  | 只有当源（Source）Actor/组件拥有**所有**这些标签时，此能力才能被激活。 |
| **Source Blocked Tags**<br>(源阻塞标签)                   | 如果源（Source）Actor/组件拥有这些标签中的**任何一个**，此能力将被阻塞。 |
| **Target Required Tags**<br>(目标所需标签)                | 只有当目标（Target）Actor/组件拥有**所有**这些标签时，此能力才能被激活。 |
| **Target Blocked Tags**<br>(目标阻塞标签)                 | 如果目标（Target）Actor/组件拥有这些标签中的**任何一个**，此能力将被阻塞。 |

#### InstancingPolicy

| Instancing Policy (实例化策略)                 | Description (描述)                                    | Details (详情)                                               |
| :--------------------------------------------- | :---------------------------------------------------- | :----------------------------------------------------------- |
| **Instanced Per Actor** (每个 Actor 一个实例)  | 为该能力创建单个实例。每次激活时都会重用该实例。      | 可以存储持久化数据。每次必须手动重置变量。                   |
| **Instanced Per Execution** (每次执行一个实例) | 每次激活时创建新实例。                                | 在激活之间不存储持久化数据。性能低于“每个 Actor 一个实例”。  |
| **Non-Instanced** (非实例化)                   | 仅使用类默认对象 (Class Default Object)，不创建实例。 | 无法存储状态，无法绑定到能力任务 (Ability Tasks) 的委托。这是三个选项中性能最好的。 |

#### Net Execution Policy

| Net Execution Policy (网络执行策略) | Description (描述)                                           |
| :---------------------------------- | :----------------------------------------------------------- |
| **Local Only** (仅本地)             | 仅在本地客户端运行。服务器不执行该能力。                     |
| **Local Predicted** (本地预测)      | 在本地客户端激活，随后在服务器激活。使用预测机制。服务器可以回滚无效的更改。 |
| **Server Only** (仅服务器)          | 仅在服务器运行。                                             |
| **Server Initiated** (服务器发起)   | 先在服务器运行，然后在拥有的本地客户端运行。                 |

`Local Only`只适合一些本地的能力视觉听觉效果

`Local Predicted`适合在较高延迟网络环境下提高游戏的流畅度

`Server Only`适合被动技能，持续监听类的技能。

`Server Initiated` 应用场景很少。

#### Replication Policy

**注意：GA默认是复制的，我们不需要修改Replication Policy.**

#### Things Not to Use: 不建议使用的事项

##### Replication Policy 复制策略

- 无用。请勿使用。有关 Epic 官方的解释，请参阅 Epic 的 Ability System Questions。
- Gameplay Abilities（游戏能力）已经会自动从服务器复制给拥有者客户端（Owning Client）。
  - 注意：Gameplay Abilities 不会在模拟代理（Simulated Proxies）上运行（请使用 GEs 和 GCs）。

##### Server Respects Remote Ability Cancellation 服务器尊重远程能力取消

- 这意味着当本地客户端（Client）的能力结束时，服务器的能力也会随之结束。
  - 通常这不是个好主意；服务器的版本（权威性）才是最重要的。

##### Replicate Input Directly 直接复制输入

- 总是将输入的按下/释放事件复制给服务器。
  - Epic 官方不鼓励这样做。

### 输入启动能力

#### 输入配置相关的Data Asset `AuraInputConfig`

我们对每个输入都配置一个标签，例如WASD，运行时我们可以把不同的标签绑定到游戏技能上，例如D按下后，每个GA都可以收到，如果一个GA包含D的标签，这个GA就可以被激活

1. 将`InputAction`和`Gameplay Tag`关联。创建一个InputConfig类Data Asset，然后再该类中创建一个结构体，该结构体包含输入动作和对应的Tag。再InputConfig类中保存该结构体的数组。并写一个根据TAG返回输入动作的查找函数。
2. 在`AuraGameplayTags`中添加原生`Gameplay Tag`
3. 在编辑器中创建对应的InputAction，修改他们的输入空间为1维。并配置这些InputAction和物理按键的映射。创建DA_InputConfig，配置不同InputAction对应的Tag。

 #### Input Component

自定义一个Enhanced Input Component组件，提供一个绑定输入动作和回调函数的函数。对于每一个输入动作绑定三个函数，分别是按下，松开，保持按下。这三个函数均接受一个Tag。这个Tag是我们在InputConfig中对每个InputAction配置的Tag。我们可以在这三个函数中查找与Tag对应的GA，并执行相关的GA。

最终我们对不同的GA配置不同的Tag就可以实现输入动作与GA的灵活配置

在PlayerController中新增InputConfig，和动作回调函数。然后调用自定义输入组件中的绑定函数，把InputConfig和动作回调函数传递给自定义输入组件。

####  实现根据输入动作的Tag查找包含相同Tag的Ability并激活该Ability

在Controller中保存ASC的唯一指针，在输入动作绑定的函数中调用ASC的函数来进行处理。

在ASC中根据输入动作包含的Tag选择包含该Tag的Ability执行。

【动态添加Tag】在ASC的AddCharacterAbilities函数中，我们需要创建AbilitySpec，并在该Spec中添加startupTag。

### 点击移动功能的实现

> 实现按住移动功能

在PlayerController添加一些和点击移动相关的辅助变量

1. 如果是左键按下，则根据光标追踪的结果设置是否存在攻击目标。并将自动寻路标识设置为False（释放的时候才进行自动寻路）
2. 如果是左键保持按下，判断是否存在攻击目标，如果存在，就执行技能（Ability）如果不存在，就开始移动，移动的时候进行光标追踪得到鼠标的世界坐标，计算角色的移动方向，调用AddMovementInput函数移动。



> 实现点击移动功能（自动寻路）

在世界中设置一个导航网格边界体

------

**Navigation System中设置允许客户端的Navigation。**

点击移动的实现思路：根据start和end的位置生成寻路点，根据这些点创建Spline线条，在每一tick中，角色寻找距离他最近的spline点，往这个点方向移动。让路径更加平滑。

![移动曲线](https://cdn.nlark.com/yuque/0/2024/png/36214189/1723048236064-b0b66e26-3f85-4edc-9080-f75556a0de9f.png)

### 发射投射物技能 

> **实现投掷物技能**

1. 创建一个投掷物类（Actor）
2. 配置ProjectileMovementComponent组件。
3. 定义一个GA类，当GA激活后在角色武器的顶端生成一个投掷物类。（c++设置）
4. GA激活后播放蒙太奇（在GA蓝图中设置）。
5. 【下面修改3，4的生成时机】
6. 在GA播放蒙太奇之后，等待GameplayEvent事件。蒙太奇的某个时间点发送GameplayEvent。
7. 为了发送GameplayEvent，需要先制作GameplayEventNotify蓝图（继承Animnotify类），在该蓝图中重写接受信息事件，在接收到信息后给Actor发送一个GameplayEvent通知。之后在蒙太奇的某个时间节点添加一个该类型的通知。这样蒙太奇发送通知的时候，就会执行GameplayEventNotify蓝图中的接收信息事件，然后发送给Actor一个GameplayEvent通知。
8. GA接收到GameplayEvent事件后（GampelayTag标识该事件），调用c++中的函数（BlueprintCallable），生成投掷物类。

#### AuraProjectile

所有投射物的基类

包含`ProjectileMovementComponent` 和 `SphereComponent`

绑定好的碰撞事件`OnSphereOverlap`

#### AuraProjectileSpell

所有发射投射物技能的基类

#### 自定义AbilityTask -UTargetDataUnderMouse

创建继承`AbilityTask`的`TargetDataUnderMouse`

让投射物能正确向目标移动

**函数**

<font style="background-color:#FBF5CB;">SpawnProjectile</font> 

- 仅服务器生成技能
  
- 从接口获取生成位置 进行延迟生成
  
- 途中设置旋转，GE

<font style="background-color:#FBF5CB;">EventActivateAbility</font> 

- 调用自写的 AbilityTask <font style="background-color:#FBF5CB;">TargetDataUnderMouse</font> 获取鼠标下的信息，等待消息的回调执行技能释放
  
- 设置运动扭曲的目标，播放施法动画，等待动画通知发送tag 而生成发送物

OnSphereOverlap

- 触碰之后播放效果。
  
- 服务器进行销毁发射物
  
- 可能存在客户端还未播放效果，导致服务器删除发射物之后，客户端没办法播放效果，因此加个标记进行**判断客户端是否已经播放效果，如果没有，则在销毁时，客户端再进行播放**



**执行** 

- 播放施法蒙太奇，等待tag的通知—— <font style="background-color:#FBF5CB;">WaitGameplayEvent</font> 

- Tag 由动画通知中重载<font style="background-color:#FBF5CB;">RecivedNotify</font> 的函数中发送(<font style="background-color:#FBF5CB;">SendGameplayTagToActor</font>)

- AbillityTask类 <font style="background-color:#FBF5CB;">UTargetDataUnderMouse</font> 获取到一些鼠标下的消息

- 重写 <font style="background-color:#FBF5CB;">Activate</font> 判断客户端与服务端，客户端直接广播  ,服务端监听之后 回调 再进行广播

- <font style="background-color:#FBF5CB;">SendMouseCursorData</font> 整个期间被预测，向服务端发送包装数据，并且代理将其广播

### AbilityTask

**AbilityTask知道自己所属的Ability**

**GATask的局限性，只在本地执行，Task中广播的数据只有本地知道。GAS内置一个在客户端与服务端传递TargetData的机制。**

#### 为了正常使用ASC的TargetData同步功能，需要在自定义AssetManager中

```c++
void UAuraAssetManager::StartInitialLoading()
{
Super::StartInitialLoading();
FAuraGameplayTags::InitializeNativeGameplayTags();
UAbilitySystemGlobals::Get().InitGlobalData();
}
```

#### 角色转向 

1. 运动扭曲技术来实现。让角色在空间中移动或者转向。
2. 启动动画的跟运动，角色增加一个motion wrap组件，需要启动插件支持。
3. 在动画蓝图中设置Wraping notify的范围，取消勾选translation，保持rotation，设置facingtarget。
4. 在角色蓝图中设置一个custom event。该事件接受一个Vector设置wrap Target Name。
5. 为了不必转换为具体的BP类，我们需要在战斗接口类中写一个蓝图实现函数来更新Wrap Target，该函数不必声明为虚函数。

### Prediction

客户端先行做出预测，然后将这个更改通知服务端，服务端确认是否合法，如果不合法，服务器撤销操作。

Prediction的关键依赖于PredictionKey，客户端会发送一个Key给服务端，其他客户端则收到无效Key，服务端接受后，判断与该Key绑定的Side Effects是否合法，如果是则告诉客户端接受该key，否则告诉客户端不接受该key，客户端要进行回滚。key无法复制。

#### GAS Automatically Predicts (GAS 自动预测的内容):

- **Gameplay Ability Activation** (游戏能力激活)
- **Triggered Events** (触发事件)
- **Gameplay Effect Application** (游戏效果应用)
  - Attribute Modifiers (not Execution Calculations) (属性修改器，不包括执行计算)
  - GameplayTag Modification (GameplayTag 修改)
- **Gameplay Cue Events** (Gameplay Cue 事件)
  - From within a predicted Gameplay Ability (来自预测的游戏能力内部)
  - Their own Events (它们自身的事件)
- **Montages** (蒙太奇动画)
- **Movement (UCharacterMovement)** (移动组件 (UCharacterMovement))

#### GAS Does NOT Predict (GAS **不**预测的内容):

- **Gameplay Effect Removal** (游戏效果移除)
- **Gameplay Effect Periodic Effects** (游戏周期性效果)

#### Ability Activation (能力激活)

**TryActivateAbility:** (尝试激活能力)

- **Client calls TryActivateAbility** (客户端调用 TryActivateAbility)
  - New `FPredictionKey` (新的 `FPredictionKey`) —— "Activation Prediction Key" ("激活预测密钥")
- **Client continues** (客户端继续执行)
  - Calls `ActivateAbility` (调用 `ActivateAbility`) —— Activation Info (激活信息)
- **Client does things** (客户端执行操作)
  - Generates side effects (产生副作用) —— 对应图中的 Side Effect (副作用，均关联预测密钥)
- **ServerTryActivateAbility** (服务器尝试激活能力)
  - Server decides if valid (服务器决定是否有效)
  - calls `ClientActivateAbilityFailed` (调用 `ClientActivateAbilityFailed`)
  - or `ClientActivateAbilitySucceeded` (或 `ClientActivateAbilitySucceeded`)
- **Client receives the Server's response** (客户端接收服务器的响应)
  - If failure, kill the ability and undo side effects (如果失败，终止能力并撤销副作用)
  - If success, side effects are valid. (如果成功，副作用有效。)
- **ReplicatedPredictionKey replicates** (ReplicatedPredictionKey 进行复制/同步)
  - `OnRep_PredictionKey`

#### Gameplay Effects (游戏效果)

- **Side effects** 
- **Only applied on Clients if:** (仅在以下情况下应用于客户端：)
  - There is a valid prediction key (存在有效的预测密钥)
- **The following are predicted:** (以下内容会被预测：)
  - Attribute Modifications (属性修改)
  - Gameplay Tag Modifications (Gameplay Tag 修改)
  - Gameplay Cues (Gameplay Cues / 游戏提示)
- **When the FActiveGameplayEffect is created** (当 FActiveGameplayEffect 被创建时)
  - Stores the Prediction Key (存储预测密钥) —— 对应的 Active Gameplay Effect 
- **On the server, it gets the same key** (在服务器上，它获得相同的密钥)
- **FActiveGameplayEffect is replicated** (FActiveGameplayEffect 被复制/同步)
  - Client checks the key (客户端检查密钥)
  - If they match, then "OnApplied" logic doesn't need to be done (如果匹配，则不需要执行 "OnApplied" 逻辑)

## Damage系统

### Meta Attributes

> 介绍

普通属性是可以复制的，元属性不会被复制，他只是个临时的占位符，我们只在服务端用它做计算，计算完后应用到真正的属性上。

> IncomingDamage

1. 在AS中设置一个Meta Attribute IncomingDamage。
2. 在PostGE函数中判断修改的属性是否是IncomingDamage属性，如果是，就处理伤害，然后把IncomingDamage属性设置为0。可以在这里判断角色是否死亡。

> 技能的伤害

1. 角色技能带有一个GESpecHandle，但是GE的伤害应该由技能指定，因此我们把GE的Modify设置为Caller，即该值由Spec直接赋值或者蓝图赋值
2. Set by Caller 的数值是成对出现的，我们需要先创建一个Tag，Damage。
3. 然后在GeSpec中添加一个键值对，Key是Damage,Value是具体的数值。外界通过key就可以拿到该Spec的伤害数值。
4. 在GE蓝图中，设置Set by Caller，同时选择对应的Tag。因为一个GE可能由多个对象设置不同的Tag和对应的value.
5. 在GA基类中存储一张表，对不同的GA设置不同的等级变化伤害。
6. 在具体的GA中配置表格。我们可以把不同的表格存放在同个Curve Table Asset中。
7. 在GA生成Spec的时候访问表格中的数据得到与当前GA等级绑定的Damage。

### Hit React

> 使用GA来实现，敌人通用

在属性值修改中 (<font style="background-color:#FBF5CB;">PostGameplayEffectExecute</font>),让血量减少伤害之后，如果没有死亡，则进行受击

通过Tag激活相应GA

GA流程：

+ 应用GE (上受击TAG)
+ 播放受击动画
+ 移除GE

1. 制作一个可以给角色添加Tag的GE。TAG为Dmage
2. 制作一个Hit React GE，授予GE Target ASC HitReact标签
3. 在敌人类中监听标签的变化（增删）。
4. 制作一个受击GA，在该GA中应用GE添加标签，之后获取不同角色的受击蒙太奇（角色转换为`ICombatInterface`类型使用接口重写蓝图原生事件实现），然后等待蒙太奇播放结束移除GE，移除受击标签。
5. 在敌人血量大于0且受击时激活受击GA。具体实现步骤如下：
6. 在CharacterClassInfo增加一个全角色均有的GA数组
7. 在蓝图可调用函数库中定义一个基于初始能力的静态函数，遍历初始能力GA。
8. 在AS中判断受击时是否死亡，如果没有就激活受击能力（根据TAG）
9. 修改GA的instance策略为Per Actor。在动画播放结束后移除 GE效果。

### Character Death

> 可以用使用GA来实现，敌人和主角通用。也可以在战斗接口中声明一个DIe虚函数，处理服务端的死亡逻辑，然后调用RPC函数处理服务端和客户端的角色死亡动画（物理效果。）。由于我们在AS中调用战斗接口的DIe函数，而AS只在服务端执行，客户端复制，因此DIE函数不需要特殊声明即可实现只在服务端被调用。

应用伤害后调用接口类的 <font style="background-color:#FBF5CB;">Die</font> 函数，AuraCharacterBase中 <font style="background-color:#FBF5CB;">Die</font> 转调 <font style="background-color:#FBF5CB;">MulticastHandleDeath</font>（<font style="background-color:#E6DCF9;">服务器进行广播执行</font>）并且 取消武器的 绑定至网格

<font style="background-color:#FBF5CB;">MulticastHandleDeath</font> 中开启布娃娃，并且替换溶解材质，调用蓝图重载的函数，改变材质参数

实现溶解效果：

1. 在敌人基类中增加MaterialInstance成员，新增一个启动时间线的蓝图实现函数，该函数根据时间返回不同的值。新增一个溶解函数。该溶解函数创建动态材质实例，动态材质的参数基于时间线设置。
2. 在多播RPC函数中调用溶解函数（同步溶解效果）
3. 注意如果为武器和角色均设置溶解效果时不能使用相同的时间线。

### 浮动伤害文字

通过`DamageTextComponent`使用`WBP_DamageText`

> Floating text

1. 创建一个Widget，设置动画，在构造函数中播放动画。
2. 创建一个WidgetComponent C++类。声明一个蓝图实现函数。设置一个该类的蓝图实例，为该Component配置第一步创建的Widget，实现蓝图实现函数（设置伤害数字）
3. 在PlayerController中配置该Component，并实现显示伤害数字函数，在该函数中检查目标Actor和Component，如果有效创建一个组件，添加到目标 Actor的跟组件上，然后立刻分离，并设置具体的伤害数值。
4. 在AS中调用PC中的显示伤害函数。
5. 组件设置延时1秒自动销毁
6. 缩放的时候不超过1就不会出现屏幕空间字体的像素化

### Execution Calculation (执行计算)EC

**UGameplayEffectExecutionCalculation**

* **Capture Attributes** 捕获属性

* 可以修改**不止一个** Attribute (属性)

* **Can have programmer logic** (可以包含程序逻辑)

* **No prediction** (无预测机制)

* **Only Instant or Periodic Gameplay Effects** 仅适用于 即时 或 周期性 Gameplay Effects 

* **捕获过程不会运行 PreAttributeChange**；因此在那里完成的任何 clamping (数值钳制/限制) 必须重新执行

* **Only executed on the Server from Gameplay Abilities **

  **with Local Predicted, Server Initiated, and Server Only **

  **Net Execution Policies** 

  - 仅在服务器 端执行，
  - 且通过Gameplay Abilities应用，
  - 并使用以下 Net Execution Policies (网络执行策略) 
    - Local Predicted (本地预测), 
    - Server Initiated (服务器发起)
    - Server Only (仅服务器) 

#### Snapshotting (Source) (快照机制 - 来源方)

*   **Snapshotting captures the Attribute value when the Gameplay Effect Spec is created**
    Snapshotting (快照) 会在 Gameplay Effect Spec创建时捕获 Attribute (属性) 值。
*   **Not snapshotting captures the Attribute value when the Gameplay Effect is applied**
    非 Snapshotting 会在 Gameplay Effect被应用时捕获 Attribute (属性) 值。
*   **From the Target, the value is captured on Effect Application only**
    对于 Target (目标)，值仅在 Effect Application (效果应用) 时捕获。

### 伤害自定义计算类EC



1. 创建一个EC类别，在GE_damage蓝图中移除Modify增加Execute
2. 在EC类重写Execute_Implementation来执行自定义计算逻辑
3. 在EC的cpp文件中，定义一个单例结构体用于存放所有需要捕获的属性，捕获属性的prop和def用 宏来简化。
4. 在Execute_Implementation中自定义计算Damage，考虑格挡率，穿透等属性。
5. 伤害计算需要的一些系数存放在一张表中，CharacterClassInfo中保存该表的指针。在蓝图可调用函数中创建一个函数返回CharacterClassInfo的指针。

### 伤害类型

> 使用Tag来实现伤害类型

1. 自定义一个伤害类型标签 和一个伤害类型标签数组
2. 派生一个自定义GA的子类DamageGA。保存一个伤害类型到伤害数值的Map。
3. 在设置伤害的时候，遍历蓝图中配置的Map
4. 获取伤害的 时候同理，遍历所有的伤害类型。

> 添加不同伤害的抗性

1. 把抗性作为属性，添加一些标签，以及属性伤害-属性抗性标签一一对应的Map。
2. 在伤害计算的时候遍历所有的伤害标签，然后根据标签捕获目标的特定抗性，
3. 应用抗性。

## GE

### **Component**

组件可赋予 GE 各种各样的行为，具体可添加的组件如下：

- `UChanceToApplyGameplayEffectComponent`：让 GE 变成概率施加；
- `UBlockAbilityTagsGameplayEffectComponent`：负责阻止基于 Gameplay Tags 激活的 Gameplay Ability，针对拥有 GE 的目标 Actor；
- `UAssetTagsGameplayEffectComponent`：让 GE 资产拥有 Tags，不会转移至任何 Actor；
- `UAdditionalEffectsGameplayEffectComponent`：添加附加的 GEs，它们可在特定条件下激活（或无条件）；
- `UTargetTagsGameplayEffectComponent`：将 Tags 授予 GE 的目标或拥有者 Actor；
- `UTargetTagRequirementsGameplayEffectComponent`：特定对目标（GE 的拥有者）Tags 的需求，并据此决定该 GE 是否施加或继续执行；
- `URemoveOtherGameplayEffectComponent`：基于特定条件，移除其他 GE；
- `UCustomCanApplyGameplayEffectComponent`：负责配置 `CustomApplicationRequirement `函数，确认该 GE 是否应被施加；
- `UImmunityGameplayEffectComponent`：负责阻止其他 GESpec 的施加；

### FAuraGameEffectContext 自定义GE上下文

原有 GEContex 无法满足现有需求（获取伤害是否暴击、格挡...），需要继承 GEContex，添加内容

函数，因 Context 改变，AbilitySystemGlobals 也得重载

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1725090408241-acf5f1b8-6a7d-4ddd-a8b6-a4526547fecc.png)

函数

+ <font style="background-color:#FBF5CB;">NetSerialize</font> 添加新变量的序列化

> 在 GEContext中增加自定义内容，例如是否暴击，是否穿透

1. 需要重写一些FGameplayEffectContext子类必须重写的虚函数。
2. 自定义一个Ability System Global Class的子类，启用这个自定义的GE上下文。重写AllocGameplayEffectContext函数。，返回我们自定义的GEContext。
3. 配置使用自定义的ASGC子类，在DefaultGame.ini中设置。或在项目设置里修改。
4. 可以在ASGC中存放任意想全局访问的数据。

## 自定义计算类 MMC
自定义计算类
1.  重写 `CalculateBaseMagnitude_Implementation `
2.  创建 `FGameplayEffectAttributeCaptureDefinition` 结构体用于捕获属性
3.  在构造函数 `UMMC_MaxHealth` 里使用 `AttributeToCapture` 捕获属性
            使用 `AttributeSource` 设置属性来源
            使用 `bSnapshot` 设置快照
4.  `FGameplayEffectAttributeCaptureDefinition` 结构体加入 `TArray` 待捕获属性数组
5.  在 `CalculateBaseMagnitude_Implementation` 中
                                 使用 `Spec.Captured(Source/Target)Tags.GetAggregatedTags` 从源/目标获取`tags`
                  创建 `FAggregatorEvaluateParameters` 并设置 `SourceTags` 和 `TargetTags`
                  使用 `GetCapturedAttributeMagnitude` 将传入的 `float` 引用修改为捕获值

`UMMC_MaxHealth` 返回 `80.f + 2.5f * Vigor + 10.f * PlayerLevel;`
`UMMC_MaxMana`返回 `50.f + 2.5f * Intelligence + 15.f * PlayerLevel;`

## 蓝图方法库AuraAbilitySystemBPLibary

<font style="background-color:#EFF0F0;">AuraAbilitySystemBPLibary</font> 继承 自 <font style="background-color:#EFF0F0;">UBlueprintFunctionLibrary</font> ，提供返回`WidgetController`的蓝图节点

+ <font style="background-color:#FBF5CB;">GetOverlayWgtController</font>从`AuraHUD` 获取 <font style="background-color:#EFF0F0;">UOverlayWidgetController</font> 实例
+ `GetAttributeMenuWidgetController`从`AuraHUD` 获取 <font style="background-color:#EFF0F0;">UAttributeMenuWidgetController</font> 实例

# GameplayTags

## AuraGameplayTags

## `AuraGameplayTags.h`

**全局单例**，包含原生`Gameplay Tags`
`InitializeNativeGameplayTags()`

- 初始化并注册 `GameplayTag`,
- 在`AuraAssetManager`中重写在引擎早期就会被调用的 `StartInitialLoading()`,并调用`InitializeNativeGameplayTags()`
- 避免需要使用`Gameplay Tags`时为空

# 特效和GamplayCues

## GamplayCues

GamplayCues实现音效、视觉效果

GameplayCue的标签必须以`GameplayCue`为顶级标签，通过tag启用，支持网络同步

> Gameplay Cue 攻击音效和受击特效、死亡音效

1. 不同角色的受击特效放用蓝图配置，战斗接口获取不同角色的受击特效。
2. 不同武器的攻击声效所不同，在TaggedMontage结构体中配置。
3. 敌人的 攻击音效一次攻击只播放一次，对于近战可以再遍历所有影响对象后再播放音效。在敌人武器处播放音效。在每个收到伤害的角色身上播放各自对应的受击特效【因为不同角色的血液颜色可能不一样】
4. 在敌人GA蓝图中生成的特效和音效不会同步到任何客户端，因为敌人的Controller是AIcontroller。因此需要使用GameplayCue来同步这些特效。
5. GameplayCue的标签必须以`GameplayCue`为顶级标签，支持网络同步
6. 更新Montage标签，每个Montage对应一个Montag标签和Socket标签，Socket标签用于标记左右手或者武器【技能起始位置】，Montage标签用于标记Montage本身以及对应的音效。
7. 采用GameplayCue蓝图实现受击特效和攻击音效。
8. 在BaseChararter中增加死亡音效指针，在角色死亡的时候播放。

> 润色其他Enmey

1. 对于远程攻击的敌方角色，在他们生成的projectileActor中配置击中特效和音效。
2. 对其他敌方角色增加击中特效，近战角色直接在蓝图中配置击中音效即可，近战角色的击中特效是受击角色的受击特效。远程角色在生成的远程投射物中配置击中音效和特效。


# MVC架构UI

![UI的MVC](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721635876296-cf0e8f22-91f8-4900-8b3c-621621ad71e4.png)

游戏UI采用MVC架构

## UAuraUserWidget-视图层

是项目中所有Wiget的基类

WidgetController 是其控制层

有设置Controller的函数 <font style="color:#ED740C;background-color:#EFF0F0;">SetWidgetController</font> ，并且在设置完成之后有函数事件 <font style="color:#ED740C;background-color:#EFF0F0;">OnWidgetControllerSetEnd</font>

### WBP_Globe_ProgressBar—玩家状态控件

编写如下父类控件，据此衍生出不同属性控件

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721659054035-8d538e70-c13f-48f5-9dd2-1cd1a161d51b.png)![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1721659076379-9e8c277c-d4f9-49f0-a82c-95a1ffb15cc4.png)

+ 为了使其子类能够任意设置大小，因此采用**<font style="background-color:#D8DAD9;">SizeBox</font>**作为底层，然后在其下套上显示区（OverLap）
+ **<font style="background-color:#D8DAD9;">Image_Background</font>** 是UI的背景图片，这里就是一个金色的圆环，设置笔刷图像，然后**绘制为图像**
+ **<font style="background-color:#D8DAD9;">ProgressBar_Globe</font>**是百分比滑条，设置为圆形，从下到上填充；该滑条不需要背景，因此在样式中将背景图的着色alpha设置为0；让滑条范围在环内，设置边距为10；
+ **<font style="background-color:#D8DAD9;">Image_Glass</font>** 为了让整个控件存在玻璃反光效果，设置笔刷就行
+ <font style="background-color:#E7E9E8;">ProgressBar_Ghost</font> 影子百分比滑条，平滑过度向目标百分比值，该滑条为背景，营造游戏UI中差值缓慢消失的效果

以上大部分内容为变量，可在子类更改

### WBP_EffectMessage

该控件为拾取道具时的播报，由Icon 和播报信息组成，并且添加了蹩脚的动画



1. 拾取道具 ->
2. 应用GE (GE设置标签) -> 
3. 绑定对自身应用GE 时广播的代理(  <font style="background-color:#E7E9E8;">OnGameplayEffectAppliedDelegateToSelf</font> ) -> 
4. <font style="background-color:#E7E9E8;">EffectAssetTagsDel</font> 广播此时Tag ->
5. 如果是相关Tag 则进行 信息播报

### WBP_Button

该项目的按钮大致一个样，因此做个基类按钮，并以此为基础派生宽按钮`WBP_WideButton`

### WBP_Overlay

将整个游戏窗口作为UI区域设置显示

+ 红蓝条的常驻显示 <font style="background-color:#EFF0F0;">WBP_Globe_ProgressBar</font> 
+ 拾取道具时添加道具拾取信息 <font style="background-color:#EFF0F0;">WBP_EffectMessage</font> 

### AuraHUD

玩家的HUD类

- 持有所有的 `WidgetController`和`WidgetControllerClass`

+ <font style="background-color:#E7E9E8;">UAuraUserWidget</font> 类选择成员 <font style="background-color:#D8DAD9;">OverlayWidgetClass</font> ，HUD将此类创建后显示输出到屏幕
+ <font style="background-color:#D8DAD9;">UOverlayWidgetController</font> 类选择器，以此创建选择类型的 单例 作为视图与模型的控制层
+ <font style="color:#ED740C;background-color:#EFF0F0;">InitOverlay</font><font style="color:#F8B881;background-color:#EFF0F0;"> </font> 函数初始化控制层实例，将widget的控制层设置为初始化的实例，并且将其输出到屏幕，调用时机为Actor初始化GAS相关组件完成之后 <font style="color:#ED740C;background-color:#EFF0F0;">AAuraCharacter::InitAbilityActorInfo()</font>
+ `Get***WidgetController()`创建并返回对应 `WidgetController`单例

### 属性菜单 Attribute Menu

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1722521687884-bf3ed417-4e9c-4e0d-a507-f732f718d867.png)

![图2 MVC属性菜单](https://cdn.nlark.com/yuque/0/2024/png/36214189/1722617212849-f1fae7c9-379c-4eba-8dc0-982646b447ee.png)

每个属性一个代理，然后通过代理广播属性的方式，来进行UI与数据的交互也还行，但是不方便维护，因为每增添一个属性，就要写更多的代码。

因此，采用一个通用代理和结构体，进行通用性广播

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1722617953989-67d073aa-d222-450c-8525-62d50372801a.png)

**<font style="background-color:#E7E9E8;">UAttributeInfo</font>** 继承自 **<font style="background-color:#E7E9E8;">UDataAsset</font>** 作为信息表管理

+ <font style="background-color:#E7E9E8;">FAuraAttributeInfo</font> 装载`Attribute`相关的数据信息
+ <font style="background-color:#FBF5CB;">FindAttributeInfoForTag</font> 通过`Tag` 查询对应的 <font style="background-color:#E7E9E8;">FAuraAttributeInfo</font> 
+ 创建 `DataAsset` 采用 <font style="background-color:#E7E9E8;">AttributeInfo</font> 类型 配置相关信息 
+ 每个属性行有自己的`Tag`，仅仅在`Tag`正确时更新自己的属性信息

## UAuraWidgetController-控制层

需要和玩家控制器、GAS、PlayerState、属性集进行交互，因此为了方便，创建了结构体 <font style="background-color:#D8DAD9;">FWidgetControllerParams</font> 储存交互类的地址

项目中**每个** <font style="background-color:#EFF0F0;">UAuraUserWidget</font> 的子类都设置了同一控制层，让控制层分别处理内部显示，并非在总集

<font style="background-color:#EFF0F0;">WBP_Overlay</font> 类下处理

**函数：**

+ 虚函数 <font style="background-color:#EFF0F0;">BroadcastInitialValues</font> 
+ 虚函数 <font style="background-color:#E7E9E8;">BindCallbackToDependencies </font> 

### UOverlayWidgetController

该类继承 <font style="background-color:#EFF0F0;">UAuraWidgetController</font> 并且使该类为蓝图类，可蓝图使用（BlueprintType,Blueprintable）

+ 定义代理，表示属性相关的值发送变化

辅助定义

+ <font style="background-color:#EFF0F0;">FUIWidgetRow</font> 继承自 <font style="background-color:#EFF0F0;">FTableRowBase</font> 让其成员可在表格行进行设置。成员为 Tag、信息、信息控件、信息图标

成员

+ <font style="background-color:#EFF0F0;">MessageWidgetDataTable</font> 表示表格，用以选择 <font style="background-color:#EFF0F0;">FUIWidgetRow</font> 相关的信息的表格 获取相关已配置的数据

函数

+ 重载函数 <font style="background-color:#EFF0F0;">BroadcastInitialValues</font> 使得属性被初始化时，广播代理

+ 重载函数 <font style="background-color:#E7E9E8;">BindCallbackToDependencies </font> 使得GAS的对应属性的代理绑定相应的值改变函数，进行相应代理的广播， 也就是广播转广播。

+ <font style="background-color:#EFF0F0;">EffectAssetTagsDel</font> 代理绑定 lambda 通过Tag查表获取信息,

      - 如果该GE 的标签是 Message 的子标签，则 <font style="background-color:#EFF0F0;">MessageWidgetRowDel</font> 进行信息广播

### UAttributeMenuWidgetController

继承 <font style="background-color:#EFF0F0;">UAuraWidgetControlle</font>  主要让属性菜单和属性数值交互。

属性菜单的`Controller`接收ASC的多个属性变化委托，发布一个携带信息的结构体的统一的委托到UI。

- 在`AttributeMenuWidgetController`声明委托，填入`FAuraAttributeInfo`结构体，发送到`ui`.
- `ui`在蓝图拆包填入。

函数

+ 重载函数 <font style="background-color:#EFF0F0;">BroadcastInitialValues</font> 通过循环遍历 <font style="background-color:#EFF0F0;">UAttributeInfo</font> 的 <font style="background-color:#EFF0F0;">AttributeInformation</font> 成员，获取每个标签，并且进行转调 <font style="background-color:#EFF0F0;">BroadcastAttributeInfo</font> 
+ 重载函数 <font style="background-color:#E7E9E8;">BindCallbackToDependencies </font> 使得GAS的对应属性的代理绑定相应的值改变函数，进行相应代理的广播， 也就是广播转广播。
+ <font style="background-color:#EFF0F0;">BroadcastAttributeInfo</font> 通过标签 ，<font style="background-color:#EFF0F0;">UAttributeInfo</font> 中找到其结构体，结构体中获取到属性之后 广播值

```c++
/* 
 * 定义 TStaticFuncPtr<T>，用来指代一种函数指针类型
 * TBaseStaticDelegateInstance 包装一个普通的 C 风格静态函数（或非成员函数），使其能够作为 UE 的委托（Delegate）来使用
 * FFuncPtr，它就是那个静态函数的原始指针类型
 * 
 * 定义了一个简短的模板别名 TStaticFuncPtr<T>，它实际代表 UE 静态委托内部所使用的原始函数指针类型。
 * 它被用来声明一个 TMap，实现从 FGameplayTag 到属性获取函数的映射，从而可以动态地通过 Tag 查找 Attribute，
 */

template <class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

//返回给 AttributeMenuWidgetController  ，将 FGameplayTag 和返回 Attribute 的静态函数指针FGameplayAttribute (*)()对应，使用时需要执行静态函数得到属性
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
```

## UI界面

### 主界面技能栏接受技能相关的UI信息

创建一个资产管理(<font style="background-color:#EFF0F0;">FAuraAbilityInfo</font>)，在里面配表，填入技能需要的相关信息

GAS执行添加技能，使控制器变更技能信息

+ 为角色添加能力时(<font style="background-color:#FBF5CB;">AddCharacterAbilities</font>)，<font style="background-color:#EFF0F0;">UAuraAbilitySystemComponent</font> 的 <font style="background-color:#CEF5F7;">AbilitiesGiveDel </font>代理进行广播，告知 <font style="background-color:#EFF0F0;">UOverlayWidgetController</font> 执行初始化技能信息(<font style="background-color:#FBF5CB;">BroadcastAbilityInfo</font>)
+ <font style="background-color:#EFF0F0;">UOverlayWidgetController</font> 添加代理(<font style="background-color:#EFF0F0;">FForEachAbility</font>)绑定函数(根据技能Tag,获取技能相关UI信息)
+ <font style="background-color:#EFF0F0;">UAuraAbilitySystemComponent</font> 执行函数(<font style="background-color:#FBF5CB;">ForEachAbility</font>)，循环获取有效的能力，执行代理(<font style="background-color:#EFF0F0;">FForEachAbility</font>)进行广播能力。代理在(<font style="background-color:#EFF0F0;">UOverlayWidgetController</font>::<font style="background-color:#FBF5CB;">BroadcastAbilityInfo</font>)中设置为了广播的Lambda表达式，<font style="background-color:#FBF5CB;">ForEachAbility</font> 相当根据能力的Tag（GetAbilityTagFromSpec获取）查找DA获取能力相关的UI信息，并且进行广播
+ UI界面中的技能球存在输入Tag，当技能信息广播时，判断相关技能是否为对应的输入Tag，如果是，则接受其信息，应用在界面上

> UI界面显示信息信息同步

1. 创建一个数据资产保存用于同步的GA图标。创建一个结构体，包含GA TAg，输入Tag，GA图标和GA背景。数据资产中包含该结构体的数组，创建一个根据GA Tag寻找结构体的函数。
2. 在OverlayController存储指向这个数据资产的指针，在编辑器中创建这个数据资产。
3. 创建GA Tag和Input Tag。
4. Control和ASC的数据交互时机：当ASC赋予完毕初始能力时候，先修改ASC中的bStartupAbilitiesGiven为true再广播。再controller中，如果ASC的bStartupAbilitiesGiven为true，则不需要绑定，直接调用函数即可，如果为false，则绑定委托再赋予初始能力后自动调用。
5. 我们不想再controller过多访问ASC中的内容，因此我们在ASC中遍历所有的可激活能力，然后广播相关信息。为了实现这一点，我们在ASC中声明一个委托A，然后再Controller定义这个委托并且绑定这个委托A，接着调用ASC中的遍历所有可激活能力的函数B，传递Controller中定义的并绑定的委托A。函数B会遍历所有的可激活能力并且广播委托A。
6. 编辑器中的UI绑定Controller中的委托，并更新自己的数据。
7. 注意ASC中的委托需要保证在服务端和客户端均被执行。在服务端，可以在基于初始能力后广播委托。在客户端，可以重写override在ASC的ActivatableAbilities绑定的网络复制回调函数（OnRep_ActivatableAbilities）中广播委托。ActivatableAbilities是一个GASpec的从其，在给予能力时，ActivatableAbilities会发生变化，因此会被同步到客户端，相应的回调函数会被激活。

### 和技能相关的UI逻辑

> 技能菜单控制器设置

1. 增加一个SpellMenuController控制器，为蓝图类型，创建控制器的蓝图。
2. 再Hud中保存全局唯一的技能菜单控制器，并增加获取技能菜单控制器的函数。再HUD蓝图中配置SpellMenuController的类类型（蓝图版）
3. 在蓝图可调用函数库中增加一个全局获取菜单控制器的函数。

> 技能菜单中的已装备界面

1. 实现方式同Overlay中的已装备技能菜单类似

> 能力类型和能力状态

1. 能力状态：锁定，带解锁，已解锁，已装备
2. 能力类型：主动，被动，None
3. 在赋予初始能力的时候给能力添加已装备的状态标签。
4. 在ASC中添加一个函数，根据GASpec返回该能力的状态标签。

> 能力状态更新

1. 在ASC中增加一个更新GA的函数，遍历数据资产中的GA，并根据GA的TAG寻找SPEC，如果没找到，并且等级符合GA的解锁条件，则创建SPec并 赋予角色该GA，修改GA的状态为Eligible。然后立刻进行能力复制

> 技能菜单按钮更新

1. 根据选中的按钮所代表的Gameplay的状态标签和当前的技能点更新菜单按钮。
2. 选中按钮后，调用controller的蓝图可调用函数，发送一个带有两个bool变量的委托
3. Widget根据bool变量设置button的状态（不可点击和可点击）。
4. 在技能状态和技能点发生变化后的回调函数中，也要更新技能菜单，即发送一个带有两个bool变量的委托

> 消耗技能点

1. 在controller中创建一个蓝图可调用函数，在按钮点击后调用这个函数
2. 步骤1的蓝图可调用函数调用 ASC的消耗技能点函数
3. 在ASC消耗技能点函数中，更新技能状态和技能等级，并广播更新后的技能等级。



> 装备技能同步更新

1. 当我们点击技能槽装备所选的技能时，会调用Controller的函数，在该函数中后先进行合规判断，被动技能只能装备到被动技能槽，之后调用服务端RPC函数更新技能状态。在服务端RPC函数的 最后，会调用客户端RPC函数更新UI。

**装配区域：**

+ 和主界面技能栏类似，通过输入标签来区分是否是该技能栏需要接管
+ 通过代理更新技能信息

**升级区域:**

+ 技能分状态，未到条件、未解锁、已解锁和已装备四种状态
+ 四种状态采用Tag进行区分,技能界面的技能也根据此来进行显示不同状态
+ 每个技能球对应一个技能Tag，接受到技能广播时，按照Tag进行筛选
+ <font style="background-color:#FBF5CB;">UpdateAbilityStatus</font> 函数会根据等级来匹配技能信息表中对应的技能，将技能添加至GAS中，然后RPC客户端广播技能状态变化。在Character中升级时进行调用

**技能选中：技能选中会导致说明栏、解锁、装备按钮的状态变化**

+ 技能球选中调用代理进行调用(<font style="background-color:#FBF5CB;">SpellGlobeSelected</font>)，根据技能标签查询技能是否在GAS中有效，技能状态标签是否占位来获取技能的标签状态
+ 根据技能点数和技能标签及技能状态标签，决定按钮的启用与否（<font style="background-color:#FBF5CB;">ShouldEnableBtns</font>）
+ 广播 <font style="background-color:#EFF0F0;">SpellGlobeSelectedDel</font> ，界面进行按钮状态更改
+ 打开菜单，技能选择框未变，但是可能因为其他情况导致技能状态改变了(升级)，导致相应的按钮无法及时更新，因此**需要在其他能更新技能状态的地方也广播按钮的改变**
+ 从技能描述的资产管理类获取技能描述信息

**技能装配**

+ 按下技能装配按钮，播放提示框动画，切换技能球按钮得取消
+ 发送选中技能的技能Tag，技能栏的装配Tag
+ 清除技能、选中技能栏 以及 技能之前的装配栏(如果存在) Tag
+ 赋予技能新的装配栏tag（选中的装配栏）,赋予装配栏新的技能信息。广播相关需要更改的信息

**技能升级：**

+ 升级时获取技能Tag，从GAS中根据tag获取Spec和status
+ 解锁改status或者升级该Spec的Level

### 主界面技能栏接受技能相关的UI信息

创建一个资产管理(<font style="background-color:#EFF0F0;">FAuraAbilityInfo</font>)，在里面配表，填入技能需要的相关信息

GAS执行添加技能，使控制器变更技能信息

+ 为角色添加能力时(<font style="background-color:#FBF5CB;">AddCharacterAbilities</font>)，<font style="background-color:#EFF0F0;">UAuraAbilitySystemComponent</font> 的 <font style="background-color:#CEF5F7;">AbilitiesGiveDel </font>代理进行广播，告知 <font style="background-color:#EFF0F0;">UOverlayWidgetController</font> 执行初始化技能信息(<font style="background-color:#FBF5CB;">BroadcastAbilityInfo</font>)
+ <font style="background-color:#EFF0F0;">UOverlayWidgetController</font> 添加代理(<font style="background-color:#EFF0F0;">FForEachAbility</font>)绑定函数(根据技能Tag,获取技能相关UI信息)
+ <font style="background-color:#EFF0F0;">UAuraAbilitySystemComponent</font> 执行函数(<font style="background-color:#FBF5CB;">ForEachAbility</font>)，循环获取有效的能力，执行代理(<font style="background-color:#EFF0F0;">FForEachAbility</font>)进行广播能力。代理在(<font style="background-color:#EFF0F0;">UOverlayWidgetController</font>::<font style="background-color:#FBF5CB;">BroadcastAbilityInfo</font>)中设置为了广播的Lambda表达式，<font style="background-color:#FBF5CB;">ForEachAbility</font> 相当根据能力的Tag查找DA获取能力相关的UI信息，并且进行广播
+ UI界面中的技能球存在输入Tag，当技能信息广播时，判断相关技能是否为对应的输入Tag，如果是，则接受其信息，应用在界面上

# MVVM UI

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1729255845852-02e194b1-9ce6-4f1f-b7ff-26a40db179a2.png)

**<font style="color:rgb(25, 27, 31);background-color:#EFF0F0;">Model-View-ViewModel</font>**<font style="color:rgb(25, 27, 31);"> 多了个</font>**<font style="color:rgb(25, 27, 31);">ViewModel</font>**<font style="color:rgb(25, 27, 31);">。项目中的菜单相关界面使用到了MVVM架构</font>

<font style="color:rgb(25, 27, 31);">上图MVC的 </font>**<font style="color:rgb(25, 27, 31);background-color:#EFF0F0;">Controller</font>**<font style="color:rgb(25, 27, 31);"> 可能变得冗余复杂，但是MVVM的 </font>**<font style="color:rgb(25, 27, 31);background-color:#EFF0F0;">ViewModel</font>**<font style="color:rgb(25, 27, 31);"> 能对其颗粒度到对应的界面，双箭头也表示双向绑定，</font>**<font style="color:rgb(25, 27, 31);">一头View，一头Modle</font>**。

和MVC很相似，有区别的地方在于，在MVC里，Controller会服务多个View，而MVVM里，每个View都拥有一个单独的ViewModel，所以ViewModel相当于精简版的Controller。


![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1729259141664-5d139ab5-51a6-44ec-a045-19ab50e97a03.png)

因此，菜单界面采用MVVM架构

**HUD基类：**ALoadScreenHUD

**ViewModel类：**UMVVM_LoadScreen（整个加载界面） ; MVVM_LoadSlot (存档槽界面)。继承UE的 **<font style="background-color:#EFF0F0;">UMVVMViewModelBase</font>**

**Wgt基类：**LoadScreenWgt

## 初始化你的Viewmodel

当你在Viewmodels窗口中点击Viewmodel时，可以通过 **创建类型（Creation Type）** 设置选择如何将它初始化。可使用以下方法：

| Viewmodel创建类型                                    | 说明                                                         |
| :--------------------------------------------------- | :----------------------------------------------------------- |
| **创建实例（Create Instance）**                      | 该控件会自动创建它自己的Viewmodel实例。                      |
| **手动（Manual）**                                   | 该控件在初始化时Viewmodel为null，你需要手动创建一个实例并为其赋值。 |
| **全局Viewmodel集合（Global Viewmodel Collection）** | 指你的项目中的所有控件均可使用的全局可用Viewmodel。需要 **全局Viewmodel标识符**。 |
| **属性路径（Property Path）**                        | 在初始化时，执行一个函数来查找Viewmodel。**Viewmodel属性路径** 将使用句点分隔的成员名称。例如：GetPlayerController.Vehicle.ViewModel。属性路径始终是相对于控件的路径。 |

## C++MVVM 使用流程

1. **定义 ViewModel**：继承 `UMVVMViewModelBase`，将需要 UI 显示的数据声明为 `UPROPERTY(FieldNotify)`。
2. **Setter 使用宏**：在 Setter 中调用 `UE_MVVM_SET_PROPERTY_VALUE`，它会自动触发 `FieldNotify`，通知所有绑定此属性的 Widget。
3. **View 绑定**：在 UMG 蓝图（Widget）中，右键属性选择“Create Binding”，将控件的属性绑定到 ViewModel 的某个 FieldNotify 属性或 Getter 函数。
4. **数据驱动 UI**：当 ViewModel 属性值变化时，绑定的 Widget 自动刷新，无需手动调用 `SetText` 等。

虚幻的MVVM方便之处在于**ViewModel**类中的变量可以设置FieldNotify 属性，在UMG中的绑定视图中，直接把该变量和UI中控件的某属性绑定

```cpp
const FString& GetLoadSlotName()const { return LoadSlotName; };

 void SetLoadSlotName(const FString& TargetName)
 {
	 UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName,TargetName);//用MVVM的宏，方便被编辑器相关模块采用
 }

/*
* FieldNotify 启用对属性的变更通知。UE5引入，更轻松地跟踪和响应属性值的变化，尤其是在数据绑定和MVVM模式下使用时。
* FieldNotify的变量 被Setter、Getter修饰，必须存在Set和Get函数
*/
UPROPERTY(EditAnywhere,BlueprintReadWrite,FieldNotify,Setter,Getter,meta=(AllowPrivateAccess="true"),DisplayName="插槽ID")
FString LoadSlotName;//存档槽的名字
```

![](https://cdn.nlark.com/yuque/0/2024/png/36214189/1729445396286-5870b6dd-83bb-48ee-9d0f-6350968b54ae.png)

如上图，该Wgt存在**LoadSlotViewModel**，能直接**让UI控件****<font style="background-color:#E7E9E8;">SlotID</font>****的****<font style="background-color:#E7E9E8;">Text</font>****变量绑定到ViewModel中被FieldNotify修饰的成员****<font style="background-color:#E7E9E8;">LoadSlotName</font>**。

### 游戏主界面和存档

> 主界面（MainMenu)

1. 创建一个主界面map，
2. 创建一个开始和结束按钮的MainMenuWidget，
3. 创建一个特殊的主界面显示的主角蓝图，在BeginPlay时播放声音，在播放声音后创建MainMenuWidget并添加到ViewPort，之后并设置UI Only模式（Widget to Focus为mainMenuWidget）和显示鼠标。
4. 在MainMenuWidget中为这两个按钮按下设置回调事件，在按下开始的时候转到Loadmenu，按下结束的时候退出游戏。
5. 在ProjectSetting中设置GameDefaultmap为main_Menu。

> Model-View-ViewModel

![img](https://pic4.zhimg.com/v2-8920456beffb90ea5924249a48a56b65_1440w.jpg)

> 加载界面（LoadScreen)

1. 初始操作
   1. 创建三个SlotWIdget：空白Slot，用于新建游戏该Widget有一个Button；Entername,用于输入角色名新建存档；TakenSlot中显示角色名字，等级，地图
   2. 创建一个LoadScreenWidgetBase类和蓝图，所有和LoadScreen相关的Widget都继承这个蓝图。
   3. 创建自定义的LoadSlotSwitcher蓝图，在LoadScreen(Overlay Widget)中使用这个蓝图。
   4. 创建一个新的HUDc++类，以及蓝图【HUD类在BeginPlay早期被调用】
   5. 从原来的GameMode c++类创建新的LoadScreenGameMode蓝图。在蓝图中配置HUD
   6. 启动UMG Viewmodel插件，创建一个MMVM_LoadScreen（MVVMViewModelBase）以及 蓝图。
2. 使用MVVM架构处理LoadMenu中UI和数据的交互【设置ViewModel】
   1. 在HUD类的BeginPlay中创建ViewModel；创建LoadMenu并添加到ViewPort；
   2. 给LoadScreen(Overlay Widget)添加ViewModel只需要在蓝图Window-ViewModel选项中添加即可【相当于添加变量】。添加的ViewModel需要一个PropertyPath【相当于变量赋值】，可以是一个Const函数，该函数返回BP_LoadScreenViewModel。【在5.3以上版本中这时候widget和ViewModel并不会双向绑定，因此如果在Widget中使用ViewModel需要手动调用PropertyPath中配置的Const函数获取ViewModel的指针。
   3. 为LoadSLotWIdget创建LoadSlotVM，（MVVM_LoadSlot)。LoadScreenVM提供创建LoadSlotVM的函数，和根据Index寻找LoadSlotVM的函数。在HUD创建LoadScreenVM后，调用该函数创建LoadSlotVM。
   4. 在LoadScreenWidget中增加一个设置VM的蓝图实现函数，在HUD完成步骤三后调用该函数为LoadScreenWidget中的子Widget设置VM。在蓝图中实现该函数，该函数初始化LoadScreenWidget的子Widget，在子Widget的初始化过程中为子Widget的子Widget设置ViewModel
   5. 注意，对Widget设置ViewModel的时候如果该Widget没有绑定VM上的值会创建失败，需要在VM设置中强制创建，相关设置遵循UE的提示即可。
3. 切换SwitchWidget中的不同子控件
   1. 在LoadSlotVM中声明一个委托，该委托携带一个switchIndex。
   2. 在LoadScreenVM中声明三个函数（函数需要接受SlotIndex），分别对应新建游戏，确定存档名字，选择存档。这些函数处理不同按钮按下的事件。
   3. 在SwitchWidget蓝图中，为子控件设置完VM后，调用子控件的初始化函数。在每个子控件的初始化函数中绑定按钮回调事件，在按钮回调事件中调用LoadScreenVM中的不同函数。最后调用自己的初始化函数。在SiwtchWidget的初始化函数中绑定LoadSLotVM中的携带SwitchIndex的委托，根据SwitchIndex切换显示不同的子控件。
4. 保存游戏对象。
   1. 创建SaveGame的子类LoadScreenSaveGame和蓝图。在GameMode类中保存LoadScreenSaveGame类类型，并创建一个保存游戏的函数，该函数接受一个MVVM_LoadSlot类型的变量和SlotIndex作为输入，即保存LoadSlotVM中的变。
   2. 只GameMode的保存函数中先根据SlotName和Index判断是否存在存档，如果存在则删除。之后创建存档，保存游戏。【注：UE中保存游戏似乎以为SlotName和SlotIndex作为存档的“主键”】
   3. 在LoadScreen_VM的NewSlotNameButtonPressed按下后调用GameMode中的保存游戏函数。
5. Widget绑定VM中的变量
   1. 在VM中对需要绑定的变量添加如下修饰UPROPERTY(EditAnywhere,BlueprintReadWrite,FieldNotify,Setter,Getter);在Setter函数中调用MVVM宏，该宏在Set变量的同时会广播委托。
   2. 在WIdget蓝图中将Text文本框或者其他组件绑定到VM中的变量
6. 从磁盘加载插槽状态
   1. 在自定义SaveGame类中增加一个状态枚举变量。
   2. 在GameMode中增加一个加载Slot的函数。
   3. 在LoadScreenVM中增加一个加载函数，该函数遍历所有的SlotVM并加载数据，如果加载成功则设置SlotVM的状态，然后调用SLotVM的初始化函数（广播数据以便显示正确的 子控件）。
   4. 在HUD函数中调用LoadScreenVM的加载函数。
   5. 在LoadScreenVM的NewSlotNameButtonPressed函数（在新建slot输入角色ID并点击确认按钮后会调用这个函数）中记得设置SlotVM的状态。
7. 存档选择和删除相关功能
   1. 选择slot后，Button将会禁用。在LoadSlotVM中创建一个广播bool的委托。在LoadScreenVM中的选择按钮按下功能后广播LoadSLotVM中的委托。在Widget中绑定该委托，并根据Bool设置Button的启用和禁用。
   2. 在选择slot之前，Play和Delete按钮应该被禁用。这就需在LoadScreenVM中广播一个SLot被选中的委托。在LoadScreenWidget的EventConstruct函数中禁用这两个按钮，在初始化函数中绑定VM的委托启用这两个按钮。
   3. 创建AreYouSureWidget，在LoadScreenWidget的删除按钮按下后把AreYouSureWidget添加到ViewPort，并禁用选择和删除按钮。AreYouSureWidget的取消按钮会发送一个取消事件，AreYouSureWidget接收到这个取消事件后启用选择和删除按钮。
   4. AreYouSureWidget的确认按钮会广播一个按钮按下事件，在LoadScreenWidget中会绑定这个按下事件，并调用VM中的删除函数。在删除函数里调用GameMode的删除slot函数。并设置slot的状态为空，调用slot的初始化函数（选择显示正确的Widget），调用slot广播EnableButton以便下次创建后选择按钮无法点击。【注意，为了确保删除的时候定位到选定的Slot，LoadScreenVM中会保存当前选中的Slot的VM指针（按下时保存】。并在删除Slot后清空 这个指针】
   5. 在AuraGameModeBase中增加一个地图Map。在LoadSlotVM中增加一个地图名的变量，并设置Setter和getter，在LoadScreenVM的新建存档函数中设置SlotVM的PlayerName，此时会广播一个PlayerName委托，在Widget中绑定VM的该委托即可。在GameMode的 保存游戏函数中同样保存 Map名字，加载存档的时候调用VM的SetMapName。
8. 存档确认加载地图功能
   1. LoadScreenWidget中的开始游戏按钮绑定点击事件，调用LoadScreenVM的开始游戏函数
   2. VM的开始游戏函数判断Selectslot是否有效，如果有效调用GameMode的加载地图函数。

# 敌人

## 敌人血条

在`AuraEnemy`添加`UWidgetComponent`

创建`WBP_ProgressBar`

将敌人自身作为`WidgetController`监听生命值发委托

## 敌人AI

普通敌人采用一个AI行为树，根据不同职业类型执行不同分支

![EnemyAI](D:\GameProject\ue\gas_aura_ll\img\EnemyAI.png)

### Next Steps

1.  **Create an AI Controller class**
    创建一个 AI Controller (AI控制器) 类
2.  **Create a Blackboard and Behavior Tree**
    创建一个 Blackboard (黑板) 和 Behavior Tree (行为树)
3.  **Add a Blackboard Component and Behavior Tree Component to the AI Controller**
    向 AI Controller (AI控制器) 添加 Blackboard Component (黑板组件) 和 Behavior Tree Component (行为树组件)
4.  **Add a Behavior Tree to the Aura Enemy**
    向 Aura Enemy  添加 Behavior Tree (行为树)
5.  **Run the Behavior Tree**
    运行 Behavior Tree (行为树)

> AI控制器，Blackboard 和行为树。基础配置

1. 在Build.cs中 添加和AI控制器相关的模块AIModule。
2. 创建自定的AI控制器 。并创建蓝图。并创建行为树的黑板的蓝图。
3. 设置敌人的Pawn控制器为 自定义的AI控制器蓝图，
4. 自定义AI控制器中包含一个行为树组件。
5. 敌人类中包含一个指向之定义AI控制器的指针，和一个行为树，在编辑器中配置行为树。
6. 在敌人类中重写 PossessBy函数，获取指向之定义AI控制器的指针。之后调用AI控制器 的黑板组件的初始化黑板方法，并运行行为树。运行的行为树是在敌人基类蓝图中配置的的行为树蓝图。

> 行为树特点

1. Selector会从**左到右**顺序执行他的子节点，一旦**成功**就返回，不会再执行其他子节点，
2. 服务一旦添加到节点上，就会按照 设定的 频率执行，只要这个分支在执行。
3. 服务可以自定义，服务的tick间隔比较长

> Service

1. 服务可以挂载到任意节点，c++中覆写的服务tick必须在蓝图中重写Recive Tick AI才能被正确执行。
2. 服务可自定义tick频率

> 行为树服务

蓝图类继承 `BTService_BlueprintBase`

> BlackBoard

1. 用于存储行为树中的变量。
2. BlackBoard中的Key可以设置是否实例同步，如果实例同步，则所有该BlackBoard实例中的值相同。

> 跟随功能

为了使得敌人跟踪转向更丝滑，在敌人蓝图中取消勾选Use Controller Rotation。使用Movement Component的自动转向。

对于GEActor额外添加一个变量来控制是否应用到敌人身上。在ApplyEffect，onbeginoverlap，endoverlap中都判断一下是否要应用到敌人身上，

> Decorator-Blackboard

1. 类似于条件判断，可以设置满足条件才继续执行
2. 可以判断Blackboard中的条件，并且监听条件，也可以设置时间条件

> Sequence

类似Selector，会从**左到右**顺序执行他的子节点，一旦**失败**就返回，不会再执行其他子节点，

> 行为树任务

1. 蓝图类继承 `BTTask_BlueprintBase`
2. 必须调用`FinishExecute`返回一个状态，否则会卡在这个任务

> 攻击后换个位置

1. 调用蓝图函数，Get Random Location In Navigable Radius。

> 敌人AI逻辑

1. 利用Service寻找最近的玩家，
2. 如果找到了最近的玩家，并且敌人没有被攻击继续执行下面的Sequence【顺序执行直到失败】
3. S1 如果敌人是远程，并且符合距离条件，就开始攻击任务（攻击任务时继续靠近）
4. S2 如果敌人是近战，并且符合距离条件，就开始攻击任务（攻击任务时继续靠近）
5. S3 如果敌人距离最近玩家的距离小于某个最大值，就追击。如果大于某个距离条件（该距离条件是S1，S2中距离条件的最小值），就自我结束。

### 环境查询系统EQS

#### 可射击位置

远程攻击的敌人攻击需要找到合适位置，并且和玩家之间没有阻隔物

1. 在角色周围生成很多Item，根据自定义规则查询每条Item的得分，保留得分最高的Item。
2. Item可以是Actor也可以是Location
3. 直接创建一个EQS蓝图，然后生成不同的Item。
4. 对不同的Item进行Trace【add Test Tracre，如果是选择留下成功Trace的，需要取消勾选Bool Match】，设置EQS Context蓝图来配置Trace目标。【重写Provice Actor】并查询所有的Character，找到AuraCharacter并返回。
5. 在此基础上基于测试距离。
6. 查询返回的是值最大的结果
7. 这里有一个小BUG，并没有忽略敌方Visibility，需要自定义EQS的 Trace过程中忽略带有EnemyTag的Actor。

## 敌人近战GA

1. 在CharacterClassInfo中增加一个新的能力数组，保存不同职业的GA
2. 修改蓝图可调用函数库中的赋予初始化能力函数，该函数接受一个枚举表示职业，然后遍历该职业的特定能力GA，并赋予给ASC。
3. 在编辑器中配置相应的数据。并创建不同职业的GA蓝图,并为GA添加GameplayTag。
4. 我们在行为树的Task中获取Pawn的ASC，然后根据GameplayTag找到对应的能力并激活。

> 敌人转向能力的实现

1. 使用motionWraping，取消勾选transformation，rotation是face。蓝图实现一个函数，接受一个Target，然后调用motionwraping的函数，执行wraping然后更新。（对应的动画要开启根骨骼运动）
2. 在敌人基类中增加一个新的变量，指向TargetActor。然后在EnemyInterface中写两个蓝图原生事件用于获取和设置这个TargetActor，接着在AuraEnemy 重写这两个蓝图原生事件。
3. 在BTT_Attack中链接BlackBoard中的target2Follow变量，然后设置敌人基类的TargetActor。
4. 在GA中调用敌人基类的Update FacingTarget后再播放攻击蒙太奇。

> 动画通知

1. 我们重用Aura Character的发送Tag的动画通知蓝图。

2. 在敌人的AM中，添加一个自定义的通知，该通知携带一个标签，GA播放蒙太奇后就直接waitGameplayEvent（指定等待的标签）。接收到标签后进行后续的伤害判定操作。

> 辅助函数

3. 在IcombatInterfac中声明几个蓝图原生函数(`BlueprintNativeEvent`)（是否死亡，获取actor)，这些函数可以直接传入上下文使用，而不必转换接口。

4. 在蓝图可调用函数库中新增球体查询Actor的功能。参考造成范围衰减伤害的内置函数`ApplyRadialDamageWithFalloff()`

5. 在DamageGameplayAbility中定义一个CauseDamage函数,实现遍历所有伤害类型`DamageTypes`，并为`GameplayEffectSpec`分配一个`SetByCaller`数值，接受一个Target，对该Target造成伤害。

6. 敌人攻击的时查询范围内的所有Actor，调用CauseDamage函数造成伤害。该GA的GE使用Aure的带有自定义计算的GE。采用caller的方式设置GE的伤害。设置伤害的时候新建一个TableCuve，在敌人GA中配置好不同伤害标签（类型）的具体伤害（伤害表格）。

7. 在显示伤害数值的时候我们将Source转换为自定义的PlayerController。此时如果Source是敌人则转换失败无法显示伤害。可以在后面添加一个将Target转换为自定义PlayerController的语句来单独处理敌人伤害显示。

> 解决敌人误伤和避让

1. 蓝图可调用函数中新增一个判断是否是友军的函数，在GA中如果是友军则不应用伤害。
2. 判断是否是友军可以使用Actor的Tag
3. 在敌人蓝图中可以搜索avoid，打开即可避让，该功能只在服务端执行，启动后会侧滑。

## 远程敌人

> Rock Projectile

1. 类似于主角的ProjectileSpell技能，为敌方角色创建一个GA蓝图【[GA_Rock](https://zhida.zhihu.com/search?content_id=269460552&content_type=Article&match_order=1&q=GA_Rock&zhida_source=entity)】，蓝图的父类为ProjectileSpell。
2. 定义一个投射物Actor。设置Static Mesh，设置最大速度和初始速度，设置启用重力。
3. 对这个Rock Projectile创建伤害曲线，并在GA_Rock中配置伤害类型和伤害数值（Scalable Float）
4. 为GA_Rock增加一个Tag，Abilities.Attack，在Data Class Info资产中配置敌方的初始GA。
5. \#####开始设置视觉效果#####
6. 创建攻击蒙太奇，配置[MotionWraping](https://zhida.zhihu.com/search?content_id=269460552&content_type=Article&match_order=1&q=MotionWraping&zhida_source=entity)，添加动画通知，发送特定标签A。在敌人角色蓝图中，配置特定标签A对应的蒙太奇，方便后续GA查，配置武器插槽方便生成Rock Actor。
7. 给弹弓做一个动画，让弹弓的皮带跟着敌人的手。首先在动画蓝图中每一帧获取角色的手部sockettransform，然后再动画中每一帧根据位置更新对应的骨骼。
8. 攻击的时候则不适用弹弓的动画，直接使用攻击的动画。为了实现这一点需要在武器ABP蓝图中增加一个变量，并设置如果变量是true，则使用步骤7，否则使用武器的默认动画，不做任何操作。
9. 在敌人攻击蒙太奇中增加两个通知分别为开始射击和结束射击，在敌人ABP蓝图中相应这两个通知，开始射击时候，获取武器的ABP，，并设置变量为False。并播放武器的射击AM。结束射击时获取武器的ABP，并设置变量为true。为了避免每帧转换对象，可以在动画初始阶段转换对象，注意动画初始事件需要先调用父类的动画初始事件。
10. \#####开始设置GE效果#####
11. GA_Rock中的蓝图链接同近战敌方GA中的类似。
12. 在RockActor的重叠事件中判断是否是友军，如果是直接返回。
13. 设置角色Mesh短时间内只能响应一次Projectile的重叠事件避免多次重复伤害

## 召唤

> 萨满召唤技能

1. 首先为召唤物(Demon)创建角色蓝图，并配置召唤物的属性：从父类蓝图和动画蓝图中创建子类，设置Mesh调整胶囊体、设置ABP蓝图中的走跑混合空间，配置武器（如有），配置socket名称，配置攻击蒙太奇和对应的标签，配置Socket Name（攻击方式，武器左右手和尾巴）。
2. 召唤的近战角色世界复用已有的近战角色的GA即可。近战攻击都是通用的。
3. 在projectile能力中，修改Spawn函数，接受一个标签，根据标签获取正确的能力（例如火球）生成位置
4. 创建一个SummonAbility c++类，该GA没有伤害，在该GA类中实现在前方指定扇形区域生成指定数量的Location的函数。并执行直线追踪得到最接近地面的位置以便在斜坡上使用。
5. 在GA里面实现**异步**生成小兵的算法：首先获得生成位置，然后在这些位置生成特效，生成特效后一个一个的生成小兵。
6. 生成小兵后需要手动设置控制器。
7. 修改被生成Actor的朝向和生成Actor保持一致。
8. 限制生成Actor的数量，在BaseCharacter中配置MinionCount，用接口获取这个值。
9. 创建一个新的BT，修改BTT_Attack，如果当前MinionCount小于临界值就激活生成Summon的能力，否则激活普通攻击的能力。
10. 在combatInterface中新增一个修改MinionCount的函数。在GA中Spawn一个随从就会对MinionCount+1
11. 生成Spawn的时候立刻绑定一个销毁事件，销毁的时候MinIonCount-1;
12. 增加点生成效果，可以在生成角色的BeginPlay中设置一个大小缩放的效果，用Timeline来辅助实现获取不同的3D缩放比例

# Gameplay

### 技能消耗和冷却

> 技能消耗

1. GA中可以设置Costs GE和CoolDown GE。
2. 在GA激活后立即Commit Ability(根据Ability Level)。如果资源不足以Cost则会阻塞技能

> Cooldown

1. 创建一个标签，Cooldown开头。
2. 创建一个持续事件的GE赋予Target一个Cooldown开头的标签。
3. 在GA中配置cooldown的GE

#### 冷却UI显示

继承 <font style="background-color:#EFF0F0;">UBlueprintAsyncActionBase</font> 实现蓝图的异步节点，通过 <font style="background-color:#EFF0F0;">UWaitCooldownChange</font> 异步任务让**UI执行**冷却相关逻辑

<font style="background-color:#EFF0F0;">UWaitCooldownChange</font> 类中的代理变量则是执行引脚，该类就 <font style="background-color:#CEF5F7;">CooldownStartDel</font>(进入冷却) 和 <font style="background-color:#CEF5F7;">CooldownEndDel</font>(完成冷却) 两个分支

**是否完成冷却：**通过目标GAS的目标Tag(冷却类Tag)数量是否为0来判断

**是否进入冷却：**通过目标GAS应用的GE是否带有冷却Tag来判断

## 经验

项目中，升级需要考虑升到下一级所需经验，升级给的点数，因此用一个DateAsset进行管理——<font style="background-color:#E7E9E8;">ULevelUpInfo</font> 

经验和等级的变化会设置 <font style="background-color:#E7E9E8;">AAuraPlayerState</font> 中的值,并且由其进行广播通知各类需要改变的内容



**如何为目标GAS添加XP的：**

+ 将添加经验视为一种被动的能力，不结束,直接等待相应的事件触发
+ 一开始便启用这种监听事件的GA(角色初始的被动能力)，GA等待事件Tag之后回调触发
+ 击杀对应的敌人后（<font style="background-color:#EFF0F0;">UAuraAttributeSet</font>::<font style="background-color:#FBF5CB;">PostGameplayEffectExecute</font> 中判断血量和伤害处），发出相应的事件Tag(<font style="background-color:#E7E9E8;">UAuraAttributeSet</font>::<font style="background-color:#FBF5CB;">SendXPEvent</font>)
+ GE应用后会修改XP这个属性(该属性是Meta类，类似Damage)，在Attribute的 <font style="background-color:#FBF5CB;">PostGameplayEffectExecute</font> 中捕捉XP的变化，并且设置给<font style="background-color:#EFF0F0;">PlayerState</font>的XP

**如何将属性点升级的:**

+ 角色开始便启用了被动监听GA(<font style="background-color:#E7E9E8;">GA_ListenForEvent</font>),该GA会等待<font style="background-color:#FBF5CB;">TagEvent</font>的信息进行执行
+ GA收到信息执行时，根据Tag注册对应的<font style="background-color:#E7E9E8;">Magnitude</font>，并且应用GE
+ GE内有各种<font style="background-color:#E7E9E8;">SetByCall</font>的<font style="background-color:#E7E9E8;">Magnitude</font>，根据标签能够设置其值，并且加上属性值

> XP总览

---

Next Steps (后续步骤)

1.  **Level Up Info Data Asset**
2.  **向 Player State 添加 XP (包含 Rep Notify )**
3.  
    **用于在 XP 变化时进行广播的 Delegate** 
4.  **Widget Controller 对 Delegate广播的响应**
5.  **XP 奖励 - 每个敌人根据其等级和职业给予的 XP 数量**
6.  **在击杀敌人时实时授予 XP**
7.  **处理 Level Up (升级) (处理潜在的连续多次升级)**
    **a. 奖励 Attribute Points (属性点)**
    **b. 奖励 Spell Points (法术点/技能点)**

---

1.  创建数据资产Level Up Info，添加根据经验选择等级的函数。在编辑器中配置数据资产。数据资产中保存的是升级到下一等级所需的累积经验。
2.  在PlayerState中增加XP和设置XP的函数，在设置XP的函数中广播XP变化，在XP属性的OnRep函数中广播XP变化（客户端广播）。并保存数据资产的指针。委托广播当前的累积XP
3.  在OverlayController中绑定PS中的委托。并发出XP变化的委托，委托发出一个Percent用于UI设置。

> 创建一个接受Gameplay Event的GA用于接受经验

1. 创建一个经验CT。在CharacterClassInfo中保存不同敌人的击杀经验。
2. 在蓝图可调用函数空中新增一个根据类别和等级获取击杀经验的函数GetXPRewardForClassAndLevel。
3. 在Combat接口类中创建一个获取类别的接口，在CharacterBase中实现这个接口。
4. 创建一个XP MetaAttribute 用于接受经验。
5. 创建一个接受经验的GA，等待Gameplay Event（标签为Attribute）接受经验的能力只在服务端运行，不需要复制,Per Actor。
6. 创建一个修改IncomingXP的GE，GE的modify由Caller设置，数据的标签为Attribute.Meta.Incoming
7. 在GA中保存该GE的类型引用，以便在GA中等待Gameplay Event后创建这个GE实例并应用到自身。
8. 在BaseCharacter中增加一个初始 被动能力数组，在ASC中赋予这个能力并启动一次。

> 发送经验Gameplay Event

1. 在AS中创建一个发送游戏事件的函数，
2. 在AS中敌方角色死亡后调用这个发送游戏事件的函数

> Level Up

1. 在AS中`HandleIncomingXP`函数判断接受经验后是否升级。因为PS依赖于AS，因此AS中如果需要访问Player State中的数据则需要通过PlayerInterface接口访问避免循环依赖。AuraCharacter实现了PlayerInterface接口，在相关函数中获取PlayerState中的数据然后返回。
2. 在AS中`HandleIncomingXP`计算得到升级数量，然后调用AuraCharacter上`PlayerInterface`，更新AuraPlayerState中的数据并广播委托。**注意获取升级奖励的时候要考虑到连续升级的情况。**
3. 在OverlayController中绑定AS的等级变化委托，在回调函数中获取最新的等级，然后广播UI相关的委托。
4. 在widget中绑定Controller的等级变化UI相关的委托，并更新UI。

## 属性

> 属性点和技能点

1. 在PlayerState中增加属性点，并在增加属性点的函数中广播新的属性点。在属性点的复制函数中同样广播新的属性点。
2. 在AttributeMenuWidgetController中定义属性点变化的委托`AttributePointsChangedDelegate`，并绑定PS中的属性点变化委托`OnAttributePointsChangedDelegate`，在该委托的回调匿名函数中广播AttributeMenuWidgetController中定义的属性点变化委托（蓝图可委派）。在MenuController的广播初始值函数中同样广播初始属性点。
3. 新建一个属性点行，添加到Menu中，在Menu中为该Widget设置控制器。在该Widget被设置控制器后，该Widget会绑定Controller的属性点变化委托，更新属性点显示UI

> 属性点升级

1. 在MenuWidget中绑定属性点变化的委托，如果属性点大于0，则启用button，否则禁用button。
2. 在MenuController中增加一个增加属性点的蓝图可调用函数`UpgradeAttribute`，该函数调用ASC中的相关函数。
3. 在MenuWidget中监听button按下事件，根据button的Tag调用MenuController中 的增加属性点的函数。
4. 在ASC中调用函数`UpgradeAttribute`与Player State解绑，判断PS中是否真的存在足够属性点，如果存在调用一个服务端RPC函数`ServerUpgradeAttribute`，在该服务端RPC函数中发送一个GameplayEvent，带上属性标签和属性magnitude。玩家有一个持续监听游戏事件的GA，可以根据游戏事件携带的Tag和magnitude创建GE并应用到自身。

> 升级后回满状态类的效果

1. 如果在AS的HandleIncomingXP中效果应用后直接设置生命值，此时最大生命值并没有因为等增加而改动，生命值只会增加到升级之前的最大生命值。
2. 我们应该在属性修改后`PostAttributeChange`判断是否是由于升级导致的最大生命值增加，如果是再设置生命值为最大生命值。

## Debuff

负面效果一定是从伤害类型的GA生成的，伤害类GA中有所需的<font style="background-color:#E7E9E8;">FDamageEffectParams</font>，在伤害类型的GA中生成伤害类型GE时，<font style="background-color:#FBF5CB;">MakeDamageEffectParamsFromClassDefaults</font> 生成其参数即可

为GE上下文添加了变量，使得debuff所需的参数传入GE上下文



蓝图库写了 <font style="background-color:#FBF5CB;">ApplyDamageEffect</font> 应用GE的函数，传入<font style="background-color:#E7E9E8;">FDamageEffectParams</font>参数调用即可

- 将参数解析，为GE的修饰器添加tag与其数值

- 在**计算伤害的类**中解析GE修饰器中的值，处理之后传入GE上下文

- 在属性集**投递伤害中**根据GE上下文判断是否应用Debuff，并且解析上下文数据，创建一个新的GE作为Debuff的GE

- 因为GE周期策略不同、防止因debuff循环调用。因此要根据GE上下文数据生成新的GE应用

---

> 技能Debuff

1. 增加和Debuff相关的标签，debuff类型，伤害，持续时间，频率，触发概率。
2. AuraAbilityTypes创建一个伤害效果参数结构体（FDamageEffectParams），伤害效果参数包含GE相关参数，目标和原ASC，Debuff相关参数。
3. 在DamageGA父类AuraDamageGameplayAbility中增加一个创建FDamageEffectParams的函数MakeDamageEffectParamsFromClassDefaults，该函数接受一个TargetActor
4. 在自己的蓝图可调用函数库中增加一个应用GE的函数ApplyDamageEffect，该函数接受FDamageEffectPara作为参数，该函数通过访问FDamageEffectParams中的信息，创建并Set by Caller修改GE中的信息，最后将创建的GE应用到FDamageEffectParams中的目标上并返回GE的SpecHandle
5. 重构ProjectileActor的重叠函数，在关联的GA中创建GE的FDamageEffectParams结构体，并传递给Actor，在Actor中填充FDamageEffectParams结构体中的目标信息，并调用蓝图可调用函数库中的函数应用GE。
6. 在自定义计算中增加处理Debuff的功能DetermineDebuff。遍历所有的伤害类型标签，获取当前的伤害类型对应的debuff标签，然后获取该 debuff的chance，最后考虑到目标的抗性后计算最终的debuff Chance，如果判定则执行下一步Debuff。
7. 在自定义GEContext中增加和Debuff、伤害类型相关的参数，并修改序列化函数。
8. 添加一些自定义的蓝图可调用函数获取GEContext中的信息。
9. 在EC计算的时候填充GEContext中与Debuff相关的信息。
10. 在AttributeSet中处理Debuff的应用。使用Dynamic GE，创建GE并对目标使用。
11. 在AS的PostGEExecute函数中，如果目标已经死亡，则直接返回不在应用伤害。



> Debuff粒子效果

1. 创建一个Component。在该component的BeginPlay中绑定所有者ASC的Tag变化标签，监听DebuffTag。
2. 为了避免ASC没有成功初始化，在character初始化后广播ASC初始化成功的委托。Component绑定该委托，在该委托的回调函数中绑定ASC的Tag变化标签。注意使用接口来的方式降低耦合。
3. Component中绑定Character死亡的委托。在Character死亡后发送委托。
4. 在Debuff标签从0变成1的时候激活niagara，死亡和标签变为0的停止激活niagara。

## 击退效果

> Death Impulse

1. 在GA中增加一个关于Death Impulse强度的变量，在GEParams结构体中也增加同样的变量，
2. 在自定义GEContext中增加Vector变量。
3. 我们要把Impulse强度和方向存储在GEContext中，在Actor碰撞后，应用GE前，需要传递这些信息，并传递给应用GE的函数 ，在GE中创建Context，并填充信息，
4. 在AS中如果死亡，则传递一个impulse向量，在死亡处理函数中设置Weapon和Mesh的 Impulse效果。

> Knock Impact

1. 同Death Impulse几乎一模一样。
2. 注意如果击退效果没有生效可以取消勾选HitResult的跟运动。
3. 在MakeDamageEffectParamsFromClassDefaults中定义覆盖knockDirection和DeathImpulse的操作。

## 高级GA

> ElectricBeam

1. 在AuraCharacter蓝图中增加一个变量bShockLoop，在ABP蓝图中每一帧都获取这个变量，根据这个变量转换Idle和ShockLoop动画状态。
2. 在接口声明一个蓝图实现函数来设置bShockLoop。
3. 在GA蓝图设置GA启动的逻辑。某些变量声明在C++中，调用C++中的函数来设置这些变量。
4. 在按下鼠标后，调用此GA，同时禁用PlayerController的某功能。在GA中设置一些标签，在GA激活后给ASC添加这些标签，在PlayerCOntroller检测这些标签并禁用某些功能。
5. 使用Gameplay Cue处理声音,需要重写On Execute函数。添加一个Gameplay Cue Tag。
6. 在GA中根据Tag执行GameplayCue。执行GameplayCue需要遍历整个项目文件，因此我们可以 吧GameplayCue存放在某个文件夹，然后在DefaultGame.ini配置GameplayCue的遍历文件夹。
7. 我们可以在DefaultEngine.ini中配置最大每次更新的最大PRC数量。每个gameplayCue会占用一个，如果GameplayCue太多，则需要多次更新才能同步。
8. 创建一个单独的trace通道

`DefaultGame.ini`

```ini
[/Script/GameplayAbilities.AbilitySystemGlobals]
+AbilitySystemGlobalsClassName=/Script/gas_aura_ll.AuraAbilitySystemGlobals
+GameplayCueNotifyPaths=/Game/Blueprints/AbilitySystem/GameplayCueNotifies
```

`DefaultEngine.ini`

```ini
[ConsoleVariables]
net.MaxRPCPerNetUpdate=10
```

> Gameplay Cue Actor

1. 需要仔细检查是否自动摧毁。
2. 在蓝图中使用AddGameplayCueOnActor(Looping)使用该Cue。
3. 在Gameplay Cue中获取武器component，然后吧Niagram Systen component附加在武器的顶端，设置末端为mouse hit result location。在GameplayCue的remove函数中销毁NiagramSystemComponent。
4. 声音同理



> 第一个追踪到的敌人

1. c++实现，GA蓝图中调用



> 寻找第一个击中敌人周围的 敌人

1. c++实现寻找周围敌人 的 方法。GA蓝图调用。
2. 对于每个周围的敌人，都增加新的GameplayCue，并保存一个Actor - Gameplay Cue Params的Map，以便在GA结束的时候移除GameplayCue.
3. 在主目标死亡后应用cool down



> 伤害相关

1. 配置好CooldownGE和CostGE
2. 生成Beam后，设置一个Timer，频率为伤害频率，在Timer的回调事件中提交GACost，如果提交成功，则应用GE。
3. 在GA中创建一个应用伤害的函数，在蓝图中创建context Spec应用伤害。关于Debuff的处理，只在最后一次应用伤害的时候判定。
4. =====如果著目标死亡的时候取消GA，如果副目标死亡，则取消应用在他身上的伤害。=====
5. 在C++中创建一个死亡后广播的委托。在BaseCharacter的MultiCastHandleDeath中广播该委托。
6. 在BeamSpell中创建两个蓝图实现的函数，并绑定主目标和副目标的死亡委托
7. 在主目标死亡后移除GameplayCue，并且清空Timer，提交冷却，结束GA。
8. 在副目标死亡后移除自身的GameplayCue，并移除AdditonalActorToCueParams中的数据以免后续继续应用伤害。
9. 最小施法时间，在鼠标释放结束后判定按下时间是否小于最小施法时间，如果小才会结束，否则延迟



> FireBolt修改Bug

在使用追踪功能后，如果敌方已经死了 ，则已经发射出去的火球会一致停留在角色死亡的位置。解决办法是在火球的低频率tick函数中判断移动距离是否小于临界值，如果小则销毁该火球。

GA中阻止所有的Ga标签

> 触电debuff

1. 在松开按键的时候，额外应用一次伤害，应用伤害 时候使用C++的函数，以便判定Debuff。
2. 在Debuff判定成功后，会给Character添加一个标签，绑定这个标签的变化事件，修改Character中的可复制变量bIsStunned，在动画蓝图中获取这个变量，并根据这个变量判断是否播放Stun动画。

> Firebolt

1. 需要设置为Replicated并且Movement也要设置Replicate

> Debuff 效果实现

1. 在角色基类中增肌一个DebuffNiagaraComponent，在构造函数中创建这个组件的默认子对象。并在子角色类的蓝图中配置这个DebuffNiagaraComponent的粒子效果。
2. DebuffNiagaraComponent是UNiagaraComponent的子类。在构造函数中取消AutoActivate。在BeginPlay函数中，获取所有者的AC，并绑定一个DebuffTag移除和增加的委托，在该委托的回调函数中判断所有者是否是有效的，所有者是否还活着，如果还活着且有效，并且是增加DebuffTag，则激活粒子效果，否则关闭粒子效果。为了避免获取不到所有者的ASC，在beginPlay中绑定所有者ASC注册好的委托，用WeakLambda绑定，在该Lambda中绑定一个DebuffTag移除和增加的委托。
3. 在角色基类的multicast_Death相关函数（处理死亡的函数客户端和服务端都会被调用）中取消激活粒子效果。
4. 如果主角的效果不会跨客户端显示，则可以在OnRep_Stunned()函数中手动调用



> 播放Shock动画

1. 在角色基类中增加一个变量表示敌方是否被麻痹，在接口中声明该变量的GetSet方法，蓝图原生方法，蓝图可调用，在角色基类中重写Get和Set方法。
2. 修改AS中的受击逻辑，如果受击并且没有被麻痹，才会尝试激活受击GE。
3. 在敌人的动画蓝图基类中，每一帧获取是否被电击的变量，如果是转移到被点击的状态。
4. 在GA能力蓝图中的SpawnElectrocute函数中，判断第一个击中目标实现了CombatInterface后，设置第一个目标的被电击变量。紧接着在获取到周围的5个Actor时，分别设置这些Actor的被电击变量。在PrepareToEndAbility函数中的移除Cue之前设置被电击变量为false。

## 被动能力

被动技能只要装配在技能槽中，就一直触发；技能同步为服务器开启，客户端也运算。

因此主要开启与关闭技能的逻辑是在技能装配时发生

+ 如果装备的插槽不是空的，则需要先清除之前的技能，如果清除的技能是被动技能，则停止技能
+ 如果装备的被动技能之前的插槽是空的，说明该技能没有处于激活状态，需要进行激活
+ 同时在此技能槽更改时，启停粒子效果

---

> 初始化被动能力，使其可以融入到现有的UI交互逻辑。

1. 创建一个新的被动能力类的基类AuraPassiveAbility，该类继承我们自己的AuraGameplayAbility。
2. 在ASC中声明一个Deactivate的委托，在PassiveAbility类中重写ActivateAbility方法，在该方法中绑定ASC中的Deactivate委托。在委托的回调函数中根据PassiveTag结束当前激活的GA。
3. 创建三个被动能力的蓝图，并填充Ability_Info资产。我们升级的时候会遍历该资产根据解锁等级解锁对应的GA并赋予ASC，然后广播解锁技能的AbilityInformation。
4. 在被动能力树中设置不同Widget的Ability Tag。在Widget中已经绑定的AbilityInfo的委托回调，根据Widget自身的Ability Tag显示对应的能力信息。
5. 技能菜单的技能槽中为不同的Widget设置不同的InputTag，这样在装备被动技能后就可以正确显示在技能槽中。
6. 在Overlay的HealthManaSpellWidget中对所有的Subwidget设置Controller和inputTag。



> 被动技能激活时机

1. 被动技能的执行策略是server initiated.
2. 被动技能在装备到技能槽后自动激活，在ASC中的服务端RPC中处理这个逻辑。点击技能槽后会调用controller的函数，在该函数中会 调用ASC的服务端RPC函数处理技能装备逻辑。在技能装备函数中需要考虑到该技能槽是否已装备了技能。同时处理前后技能的状态（已解锁--已装备）



> 被动技能装备的 Niagara粒子效果

1. 创建一个 UPassiveNiagaraComponent类，在该类的BeginPlayer中绑定ASC中的Passive激活相关的委托，绑定的方法同DebuffNiagaraSystem一致。
2. 在绑定的回调函数中，根据Tag判断是哪一个Passive，然后处理激活和取消激活逻辑。
3. 在ASC中定义一个NetMulticast函数广播Passive激活相关的委托。在ASC中的ServerEquipAbility函数的合适时机调用NetMulticast函数。
4. 在Character中创建三个这样的PassiveNiagaraSystem组件 。把该组件添加到ScentComponent组件上，并启用Tick，设置ScentComponent的旋转向量为0。

## AOE技能

元素水晶从地面生成，以生成点为中心，造成径向伤害

**法阵指示物 AMagicCircle：**

+ 透贴材质，跟随鼠标移动
+ 于 <font style="background-color:#EFF0F0;">AuraPlayerController</font> 中添加与销毁

**生成点位 APointCollection：**

+ 生成点位是提前设置好相对位置的Actor
+ 能够根据输入参数进行旋转，获取点位到地面的投影位置

**技能触发：**

+ 第一次触发技能生成法阵指示物，标识生成区域
+ 第二次触发释放技能，使用Timer进行异步生成水晶
+ 在水晶生成的位置添加径向伤害，已在 <font style="background-color:#EFF0F0;">AuraDamageGameplayAbility</font> 中添加径向伤害相关的属性
+ 如果开启了径向伤害，击退逻辑已经改为根据径向原点计算，伤害径向化在 <font style="background-color:#EFF0F0;">UExecCalc_Damage</font> 计算每项伤害时进行
+ **径向原点必须在GA生成伤害参数( ****<font style="background-color:#FBF5CB;">MakeDamageEffectParamsFromClassDefaults </font>****)之前调用！！！**

---

**GameplayCueNotify_Static**适用于简单的、不需要复杂逻辑的 单次效果
**GameplayCueNotify_Burst** 是 GameplayCueNotify_Static 的一个增强版本，用于 瞬时的“爆发式”效果，并且支持更多的细节配置。
**GameplayCueNotify_HitImpact** 专门用于处理 击中（Hit）事件 的效果。它通常用来处理攻击命中目标后的反馈，如产生火花、击退、流血等效果。

---

> 生成魔法圈，显示，隐藏和跟随鼠标。【PlayerController--MagicCircle】

1. 自定义一个MagicCircleActor，配置一个UDecalComponent组件，在蓝图中配置UDecalComponent的Material。并修改朝向【打到墙上还是打到地面】。在Tick的时候修改X旋转变量实现旋转效果。
2. 我们把MagicCircleActor保存在PlayerController中，任何与MagicCircleActor相关的操作都需要调用PC的函数。然后再包装一层，通过PlayerInterface中的函数来访问PlayerController中的函数。最后在AuraCharacter中实现PlayerInterface的接口。
3. 在PlayerController中增加一个MagicCircleActor Class，和一个MagicCircleActor指针。增加显示和隐藏函数（蓝图可调用），在显示函数中生成Actor并根据传递的Material更新Actor的Material。在隐藏函数中销毁Actor。
4. 为了解决当鼠标移动到敌人身上时，MagicCircle会突然移动，我们自定义一个新的碰撞通道，角色，敌人的Capsule，Mesh，Weapon会忽略这个通道。在MyAura.h中声明这个自定义通道的名字。在Player的CUrsorTrace函数中，在进行光标追踪之前，如果MagicCircle是合法的则使用自定义通道，否则使用Visible通道。注意需要确保空气墙忽略这些和技能相关的自定义通道。

> Shard技能配置

1. 同前面一致不再赘述
2. SpellDirection
   1. 为该技能创建一个单独的类，填充对应的Description

> Shard 技能特效和伤害逻辑

1. 能力激活显示MagicCircle，直接调用PlayerInterface中的函数即可（蓝图 原生事件，蓝图可调用函数）。之后等待激发该GA的按键再次按下，隐藏MagicCircle生成一些晶体，并处理伤害。
2. 我们想在MagicCircle中选择不同的数据点来生成晶体。为了实现这一点，我们创建一个PointCollectionActor，在PointCollectionActor中手动固定一些SceneComponent，这些SceneComponent的位置在蓝图中固定。在 PointCollectionActor 中声明一个蓝图原生函数返回给定数目的位置（SceneComponent的位置），返回位置的时候进行线性追踪得到距离地面最近的点。
3. 在GA中的合适时机生成这个PointCollectionActor并调函数得到一些点位并保存这些点位，然后启动一个Timer，每隔一段时间生成一个晶体。
4. 创建一个GameplayCue(Brust)来生成晶体，BrustEffect中的粒子效果生成位置来源于Param中的Location
5. Montage
   1. 添加一个Tag，Montag中发送MontageEvent。
   2. 在获取到所有生成晶体的点位后，调用PlayMontageAndWait之后立刻WaitGameplayEvent，在接收到EventTag之后执行后续的操作
6. 经向放射衰减伤害
   1. 为了使用经向放射性伤害相关的函数，需要在自定义的DamageEffectParams中增加四个变量【是否是经向衰减伤害，内半径，外半径，原点】并在自定义的GEContext中 同样增加这四个变量【别忘了set和get】，并修改网络序列化函数。在GADamage基类中同样在增加这四个变量，在MakeDamageEffectParams相关函数中将GADamage中这四个变量的值赋值给DamageEffectParams中对应变量的值。
   2. 在BlurFuncLibrary中定义上述四个变量的Set和Get静态函数。在ApplyDamag中调用Set函数将DamageEffectParams中这四个变量的值赋值给GEContext中对应的变量。
   3. 为了在自定义伤害计算类中获取到衰减后的伤害，需要如下几个步骤
      1. 在CombatInterface中创建一个委托，在角色基类中重写TakeDamage函数，广播接收到的Damage。
      2. 在CombatInterface中写一个函数返回这个委托，解绑EC和Character。
      3. 在EC中会遍历所有类型的伤害然后累加【如果伤害为0，则continue】，在累加前判定是否是经向衰减伤害，如果是则获取Target上的TakeDamage委托，并绑定一个Lambda函数更新伤害。之后调用 ApplyRadialDamageWithFalloff开始计算经向衰减后的伤害。
   4. 在蓝图中应用经向伤害
      1. 在GA蓝图中配置内圈半径和外圈半径，并在生成晶体的Event中设置Origin。
      2. 在生成晶体的时候【步骤3】获取当前晶体周围的所有Actor，然后调用MakeDamageEffectParamsFromClassDefaults函数创建DamageEffectParams并调用自定义的蓝图可调用 函数库静态函数ApplyDamageEffect应用伤害。
7. Cost And Cooldown
   1. 配置好这两个GE，在ABilityInfo中填充CooldownTag
   2. 在生成第一个晶体后提交Cost，在激活能力最初检测Cost
   3. 在生成最后一个晶体后提交Cooldown。



注意在蓝图的Loop中，如如果数据来源来自BlueprintPureFunction，则 每次loop的时候都会重新调用一次该BlueprintPureFunction，如果在该BlueprintPureFunction中使用了随机函数，则更加复杂。因此如果我们希望在BlueprintPureFunction中使用固定的一个随机数，我们需要保存一下BlueprintPureFunction的返回值，然后再Loop，以免每次Loop的时候都重新设置新的随机数。

### FireBlast!

> 初始操作

1. 创建一个DamageGA的子类FireBlast，增加一个变量NumBall表示火球数量。创建该GA的蓝图并分配GameplayTag。
2. 在AbilityData资产中增加该GA的信息。
3. 将OffensiveTree的某个Widget的Tag设置为该GA的Tag
4. 创建CostGE和CooldownGE，并在GA中配置他们

> Spawn Actor

1. 基于AuraProjectiles创建一个新的Actor，FireBall。在beginPlay中调用父类的beginPlay初始化一些模块，重写父类的Overlap方法。创建FireBall蓝图，并配置相关的视觉和声音效果，禁用Movementcomponent（Start with tick enable 为false，auto activate 为false）
2. 在GA_FireBlast中保存FireBall类类型，并提供一个SpawnFireBall蓝图可调用函数。

> 火球飞出去在飞回来（需要实时确定Avatar的位置）并且爆炸。用Timelines实现

1. 在FireBall Actor中增加Avatar的指针，在GA中生成Actor的时候保存这个指针。
2. 在FireBall Actor中增肌一个开始Timelin 的蓝图可实现事件，并在BeginPlay中调用这个事件。
3. 在StartTimeline函数中，如果是服务器则存储其实位置和结束位置结束位置根据Distance和朝向计算得到，之后启动Timeline插值更新Actor的位置，Timeline结束后再启动Timeline回到Avatar身边。在第二次Timeline的时候判断Actor的位置和Avatar的距离是否小于临界值，如果是就调用该Actor父类的OnHit函数播放爆炸效果和声音，最后调用销毁函数

> 造成伤害

1. 在GA中设置伤害类型标签，配置伤害ScalableTable，取消KnockBack功能。
2. 在Actor的Overlap事件中获取目标ASC，在DamageEffectParams中设置DeathImpulse（和Actor重叠时候的位置相关），最后调用BlueFuncLibrary::ApplyDamageEffect。
3. 实现爆炸伤害：
   1. 在Actor中创建一个受保护的ExplosionDamageParams。在蓝图可调用函数中声明一些修改DamageParams中内容的静态函数以便在蓝图中调用。
   2. 在SpawnActor的时候初始化ExplosionDamageParams
   3. 在蓝图中Actor爆炸后，获取周围的Actor，然后设置ExplosionDamageParams中和RadialDamage,KnockImpulse,DeathImpulse相关的参数，并对周围的Actor应用伤害。

# 调试

### 自定义日志

在项目根目录创建两个文件

```cpp
.h
#pragma once
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAura, Log, All);

.cpp
#include "AuraLogChannels.h"

DEFINE_LOG_CATEGORY(LogAura)
```

# UE 5.6 编译错误记录

## Git 中文文件名

### 错误现象
使用 UnrealBuildTool 编译项目时，出现以下异常：

```bash
UnrealBuildTool failed with exit code 0xe0434352
```

退出码 `0xe0434352` 为 Windows .NET CLR 未处理异常，说明 UnrealBuildTool（UBT）自身进程崩溃，而非源码编译错误。

### 排查过程

#### 第一步：查看 UBT 日志

日志路径：%localappdata%\UnrealBuildTool\Log.txt

日志在以下位置**戛然而止**，无任何异常信息：

```
Using EngineIncludeOrderVersion.Unreal5_6 for target gas_aura_llEditor.Target.cs 
```

#### 第二步：手动执行 UBT 获取完整异常

通过 PowerShell 直接运行 UBT，在控制台捕获完整的 .NET 异常堆栈：

```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" `
  gas_aura_llEditor Win64 DebugGame `
  -Project="D:\GameProject\ue\gas_aura_ll\gas_aura_ll.uproject" `
  -NoMutex
```

#### 第三步：分析异常堆栈

控制台输出了完整异常：

```bash
PS D:\GameProject\ue\gas_aura_ll> dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" `
>>   gas_aura_llEditor Win64 DebugGame `
>>   -Project="D:\GameProject\ue\gas_aura_ll\gas_aura_ll.uproject" `
>>   -NoMutex
Deprecated setting found in "C:\Users\long liu\AppData\Roaming\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml":
The setting "bAllowUBALocalExecutor" is deprecated. Support for this setting will be removed in a future version of Unreal Engine.
Log file: C:\Users\long liu\AppData\Local\UnrealBuildTool\Log.txt
Deprecated setting found in "C:\Users\long liu\AppData\Roaming\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml":
The setting "bAllowUBALocalExecutor" is deprecated. Support for this setting will be removed in a future version of Unreal Engine.
Using 'git status' to determine working set for adaptive non-unity build (D:\GameProject\ue\gas_aura_ll).
Creating makefile for gas_aura_llEditor (no existing makefile)
Unhandled exception. System.ArgumentException: Path fragment '"img/\346\225\214\344\272\272AI.png"' contains invalid directory separators.
   at EpicGames.Core.FileSystemReference.CombineStrings(DirectoryReference baseDirectory, String[] fragments) in D:\build\++UE5\Sync\Engine\Source\Programs\Shared\EpicGames.Core\FileSystemReference.cs:line 80
   at EpicGames.Core.FileReference.Combine(DirectoryReference baseDirectory, String[] fragments) in D:\build\++UE5\Sync\Engine\Source\Programs\Shared\EpicGames.Core\FileReference.cs:line 151
   at UnrealBuildTool.GitSourceFileWorkingSet.AddPath(String Path) in D:\build\++UE5\Sync\Engine\Source\Programs\UnrealBuildTool\System\SourceFileWorkingSet.cs:line 276
   at UnrealBuildTool.GitSourceFileWorkingSet.OutputDataReceived(Object Sender, DataReceivedEventArgs Args) in D:\build\++UE5\Sync\Engine\Source\Programs\UnrealBuildTool\System\SourceFileWorkingSet.cs:line 242
   at System.Diagnostics.AsyncStreamReader.FlushMessageQueue(Boolean rethrowInNewThread)
--- End of stack trace from previous location ---
   at System.Threading.ThreadPoolWorkQueue.Dispatch()
   at System.Threading.PortableThreadPool.WorkerThread.WorkerThreadStart()
PS D:\GameProject\ue\gas_aura_ll>
```

异常明确指向：GitSourceFileWorkingSet.AddPath 在解析 git status 输出时，遇到了无法处理的路径格式。

### 根本原因
1. **Git 工作区中存在包含中文字符的未跟踪文件**（如 `img/敌人AI.png`）。  
2. UnrealBuildTool 通过 `git status` 检测文件变更时，Git 默认将中文字符转为八进制转义序列（如 `\346\225\214\344\272\272AI.png`）。  
3. UBT 在解析该路径并写入 UHT 的 JSON manifest 时，`System.Text.Json` 无法正确处理某些转义序列，引发 `IndexOutOfRangeException`。  
4. 即使删除了中文文件，Git 暂存区或索引中可能仍残留该文件的记录（`new file:` 与 `deleted:` 并存状态），导致 UBT 持续崩溃。

触发链路

```
修改了中文命名的文件
        ↓
git status 将其列入变更文件列表
        ↓
git 默认开启 core.quotepath=true
将非 ASCII 字符转义为八进制序列并加引号
输出：'"\346\225\214\344\272\272AI.png"'
        ↓
UBT 的 GitSourceFileWorkingSet 逐行解析 git status 输出
        ↓
路径解析器遇到首尾引号，误判为非法路径分隔符
        ↓
抛出 System.ArgumentException → UBT 进程崩溃
        ↓
exit code 0xe0434352
```

### 解决方案

####  清理 Git 状态
```bash
# 重置所有本地未提交更改，清除暂存区记录
git reset --hard HEAD
# 删除所有未跟踪的文件（包括中文文件）
git clean -fd
```

#### 方案一：关闭 git quotepath（推荐，立即生效）

在项目目录执行（仅对当前仓库生效）：

```bash
git config core.quotepath false
```

或全局生效：

```bash
git config --global core.quotepath false
```

配置后重新编译即可恢复正常，无需修改任何项目文件。

#### 方案二：重命名文件（治本）

将涉及的中文文件名改为英文或拼音

#### 方案三：将文件加入 .gitignore（规避）

若该配置文件属于本地个人配置，不需要被 git 追踪：

文件从 git 追踪中移除后，不再出现在 git status 输出中，UBT 也就不会解析到它。



### 遇到新的错误

```bash
 Compile [x64] SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.cpp: Exited with error code -1 . The build will fail.
```

#### 禁用 UBA 并获取详细编译错误

```bash
# 在项目目录下执行
Remove-Item -Recurse -Force Intermediate, Binaries -ErrorAction SilentlyContinue

dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" `
  gas_aura_llEditor Win64 Development `
  -Project="D:\GameProject\ue\gas_aura_ll\gas_aura_ll.uproject" `
  -NoUBA
```

输出 **UBT 内部的 JSON 序列化异常**

```bash
Unhandled exception: IndexOutOfRangeException: Index was outside the bounds of the array.
   at System.Text.Json.JsonWriterHelper.EscapeString(ReadOnlySpan`1 value, Span`1 destination, Int32 indexOfFirstByteToEscape, JavaScriptEncoder encoder, Int32& written)
   at System.Text.Json.Utf8JsonWriter.WriteStringEscapeValue(ReadOnlySpan`1 value, Int32 firstEscapeIndexVal)
   ...
```

### 解决方案

尝试编辑 `%AppData%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`，替换为以下内容：

```xml
<?xml version="1.0" encoding="utf-8"?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
    <BuildConfiguration>
        <!-- 禁用 UBA（Unreal Build Accelerator） -->
        <bAllowUBAExecutor>false</bAllowUBAExecutor>
        <!-- 禁用并行 UHT（避免多线程序列化冲突） -->
        <bAllowUHTParallelization>false</bAllowUHTParallelization>
    </BuildConfiguration>
</Configuration>
```

保存后重新编译。

### 预防建议
- 项目路径、源码、资产名称中**避免使用中文、空格或特殊符号**。
- 使用 Git 时设置 `git config --global core.quotepath false` 可让中文正常显示，但最稳妥的方式是从源头禁用非 ASCII 文件名。
- 若编译问题与 UBT 内部 JSON 操作相关，优先尝试禁用 `bAllowUHTParallelization`。

### 参考
- 错误日志路径：`C:\Users\<用户名>\AppData\Local\UnrealBuildTool\Log.txt`
- 社区讨论：[CSDN 类似问题](https://blog.csdn.net/weixin_42409793/article/details/161228636)
