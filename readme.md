

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

## GE



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

# 敌人血条

在`AuraEnemy`添加`UWidgetComponent`

创建`WBP_ProgressBar`

将敌人自身作为`WidgetController`监听生命值发委托
