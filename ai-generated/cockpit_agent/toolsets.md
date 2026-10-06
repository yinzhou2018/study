# Toolsets

| Toolset | Name | Tools | Description |
|---|---|---:|---|
| `toolset_body_control` | 车身控制工具集 | 21 | 车身机械执行部件的运动控制，包含车窗、天窗、门锁、雨刮、后视镜等 |
| `toolset_lighting_control` | 灯光照明工具集 | 16 | 全车内外灯光系统控制，包含大灯、氛围灯、迎宾灯、星空顶等 |
| `toolset_seat_system` | 座椅系统工具集 | 13 | 全车座椅全维度调节与舒适功能，包含加热、通风、按摩、记忆等 |
| `toolset_climate_control` | 空调温控工具集 | 15 | 空调系统核心温湿度控制与空气净化 |
| `toolset_cabin_comfort` | 座舱舒适配置工具集 | 10 | 座舱专属舒适配置，包含冰箱、香氛、天幕遮阳等独立功能 |
| `toolset_media_entertainment` | 影音娱乐工具集 | 13 | 全车影音媒体播放与音量控制 |
| `toolset_navigation` | 导航出行工具集 | 8 | 导航路径规划、POI搜索、路况与沿途服务查询 |
| `toolset_vehicle_info` | 车辆信息工具集 | 4 | 车辆内外各类只读状态信息查询 |
| `toolset_charging_management` | 充电管理工具集 | 6 | 新能源车辆端充电控制与管理 |
| `toolset_communication` | 通讯通话工具集 | 5 | 车载电话、短信、通讯录管理 |
| `toolset_driving_assist` | 驾驶辅助工具集 | 12 | ADAS驾驶辅助、泊车控制、显示设置 |
| `toolset_cabin_modes` | 座舱模式工具集 | 8 | 高频场景一键触发，内部调用各工具集原子工具执行 |

## 车身控制工具集

- **Toolset ID:** `toolset_body_control`
- **Description:** 车身机械执行部件的运动控制，包含车窗、天窗、门锁、雨刮、后视镜等

| Function | Description |
|---|---|
| `ctrl_window` | 车窗玻璃开度调节，按位置单独或全车联动控制 |
| `ctrl_sunroof_tilt` | 天窗翘角通风模式开关 |
| `ctrl_sunroof_open` | 天窗全开滑动控制 |
| `ctrl_sunroof_shade` | 天窗遮阳帘开度控制 |
| `ctrl_door_lock` | 全车中控门锁控制 |
| `ctrl_child_lock` | 后排儿童锁开关 |
| `ctrl_tailgate` | 电动尾门开关控制 |
| `ctrl_door_soft_close` | 电吸门辅助关闭 |
| `ctrl_door_handle` | 电动门把手弹出/收回 |
| `ctrl_mirror_fold` | 外后视镜折叠控制 |
| `ctrl_wiper` | 前后风挡雨刮档位控制 |
| `query_window_status` | 查询四门车窗玻璃当前开度百分比 |
| `query_sunroof_tilt` | 查询天窗翘角通风模式状态 |
| `query_sunroof_open` | 查询天窗滑动开启百分比 |
| `query_sunroof_shade` | 查询天窗遮阳帘开度百分比 |
| `query_door_lock_status` | 查询全车中控门锁锁定状态 |
| `query_child_lock_status` | 查询后排儿童锁开关状态 |
| `query_tailgate_status` | 查询电动尾门当前状态（开/关） |
| `query_door_handle_status` | 查询前左/前右电动门把手弹出状态 |
| `query_mirror_fold_status` | 查询外后视镜折叠状态 |
| `query_wiper_status` | 查询前/后风挡雨刮当前档位 |

### Tool details

#### `ctrl_window`

车窗玻璃开度调节，按位置单独或全车联动控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right, rear_left, rear_right, all | `` | 控制位置：左前/右前/左后/右后/全车 |
| `openness` | `integer` | No | 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 | `50` | 车窗开度百分比，0=全关，100=全开，步长5% |

#### `ctrl_sunroof_tilt`

天窗翘角通风模式开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭翘角模式 |

#### `ctrl_sunroof_open`

天窗全开滑动控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `openness` | `integer` | No | 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 | `50` | 天窗开启百分比，步长5% |

#### `ctrl_sunroof_shade`

天窗遮阳帘开度控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `openness` | `integer` | No | 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 | `50` | 遮阳帘开度百分比，步长5% |

#### `ctrl_door_lock`

全车中控门锁控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | lock, unlock | `` | 上锁/解锁 |

#### `ctrl_child_lock`

后排儿童锁开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭儿童锁 |

#### `ctrl_tailgate`

电动尾门开关控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `action` | `string` | Yes | open, close | `` | 打开/关闭尾门 |

#### `ctrl_door_soft_close`

电吸门辅助关闭

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right, tailgate | `` | 控制位置：左前/右前/尾门 |

#### `ctrl_door_handle`

电动门把手弹出/收回

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right | `` | 控制位置：左前/右前 |
| `status` | `string` | Yes | pop, retract | `` | 弹出/收回 |

#### `ctrl_mirror_fold`

外后视镜折叠控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | fold, unfold | `` | 折叠/展开 |

#### `ctrl_wiper`

前后风挡雨刮档位控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front, rear | `` | 控制位置：前/后风挡 |
| `level` | `string` | Yes | off, 1, 2, 3, auto | `` | 雨刮档位 |

#### `query_window_status`

查询四门车窗玻璃当前开度百分比

No parameters.

#### `query_sunroof_tilt`

查询天窗翘角通风模式状态

No parameters.

#### `query_sunroof_open`

查询天窗滑动开启百分比

No parameters.

#### `query_sunroof_shade`

查询天窗遮阳帘开度百分比

No parameters.

#### `query_door_lock_status`

查询全车中控门锁锁定状态

No parameters.

#### `query_child_lock_status`

查询后排儿童锁开关状态

No parameters.

#### `query_tailgate_status`

查询电动尾门当前状态（开/关）

No parameters.

#### `query_door_handle_status`

查询前左/前右电动门把手弹出状态

No parameters.

#### `query_mirror_fold_status`

查询外后视镜折叠状态

No parameters.

#### `query_wiper_status`

查询前/后风挡雨刮当前档位

No parameters.
## 灯光照明工具集

- **Toolset ID:** `toolset_lighting_control`
- **Description:** 全车内外灯光系统控制，包含大灯、氛围灯、迎宾灯、星空顶等

| Function | Description |
|---|---|
| `ctrl_headlight_mode` | 大灯模式控制 |
| `ctrl_drl` | 日行灯开关 |
| `ctrl_fog_light` | 雾灯开关 |
| `ctrl_ambient_light` | 车内氛围灯控制 |
| `ctrl_ambient_scene` | 氛围灯场景模式 |
| `ctrl_welcome_light` | 迎宾灯光系统（车外+车内联动） |
| `ctrl_star_roof` | 星空顶控制 |
| `ctrl_interior_reading` | 车内阅读灯/顶棚灯 |
| `query_headlight_mode` | 查询大灯当前工作模式 |
| `query_drl_status` | 查询日行灯开关状态 |
| `query_fog_light_status` | 查询雾灯开关状态 |
| `query_ambient_light_status` | 查询氛围灯当前颜色/亮度 |
| `query_ambient_scene` | 查询氛围灯当前场景模式 |
| `query_welcome_light_status` | 查询迎宾灯开关状态 |
| `query_star_roof_status` | 查询星空顶当前亮度/动态模式 |
| `query_reading_light_status` | 查询前左/前右/后排阅读灯各位置状态 |

### Tool details

#### `ctrl_headlight_mode`

大灯模式控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | off, auto, low_beam, high_beam, adaptive_matrix | `` | 大灯工作模式 |

#### `ctrl_drl`

日行灯开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_fog_light`

雾灯开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_ambient_light`

车内氛围灯控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `color` | `string` | Yes | red, crimson, maroon, brick, scarlet, wine, coral, ruby, orange, amber, tangerine, apricot, sunset, yellow, gold, lemon, mustard, honey, green, lime, olive, mint, emerald, jade, forest, sage, cyan, teal, turquoise, aqua, spring, blue, navy, sky, azure, cobalt, sapphire, indigo, steel, purple, violet, lavender, plum, orchid, magenta, pink, rose, salmon, peach, fuchsia, brown, chocolate, coffee, tan, white, black, gray, silver, ivory, snow, ash, slate, charcoal, fog | `` | 灯光颜色（64色可选） |
| `brightness` | `integer` | Yes |  | `` | 亮度百分比 |

#### `ctrl_ambient_scene`

氛围灯场景模式

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `scene` | `string` | Yes | driving, rest, welcome, music_sync | `` | 场景模式 |

#### `ctrl_welcome_light`

迎宾灯光系统（车外+车内联动）

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_star_roof`

星空顶控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `brightness` | `integer` | Yes |  | `` | 亮度百分比 |
| `mode` | `string` | Yes | static, breathe, meteor | `` | 动态模式 |

#### `ctrl_interior_reading`

车内阅读灯/顶棚灯

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right, rear, all | `` | 控制位置：左前/右前/后排/全车 |
| `status` | `string` | Yes | on, off, auto | `` | 开关状态 |

#### `query_headlight_mode`

查询大灯当前工作模式

No parameters.

#### `query_drl_status`

查询日行灯开关状态

No parameters.

#### `query_fog_light_status`

查询雾灯开关状态

No parameters.

#### `query_ambient_light_status`

查询氛围灯当前颜色/亮度

No parameters.

#### `query_ambient_scene`

查询氛围灯当前场景模式

No parameters.

#### `query_welcome_light_status`

查询迎宾灯开关状态

No parameters.

#### `query_star_roof_status`

查询星空顶当前亮度/动态模式

No parameters.

#### `query_reading_light_status`

查询前左/前右/后排阅读灯各位置状态

No parameters.
## 座椅系统工具集

- **Toolset ID:** `toolset_seat_system`
- **Description:** 全车座椅全维度调节与舒适功能，包含加热、通风、按摩、记忆等

| Function | Description |
|---|---|
| `ctrl_seat_driver_memory` | 主驾座椅记忆位置调用 |
| `ctrl_seat_heat` | 座椅加热档位控制 |
| `ctrl_seat_vent` | 座椅通风档位控制 |
| `ctrl_seat_massage` | 座椅按摩控制 |
| `ctrl_seat_driver_lumbar` | 主驾腰托调节 |
| `ctrl_seat_boss_key` | 老板键（副驾座椅前移） |
| `ctrl_seat_rear_recline` | 后排座椅靠背角度调节 |
| `query_seat_heat_status` | 查询主驾/副驾/后排座椅加热当前档位 |
| `query_seat_vent_status` | 查询主驾/副驾/后排座椅通风当前档位 |
| `query_seat_massage_status` | 查询主驾/副驾按摩当前模式与强度 |
| `query_seat_lumbar_status` | 查询主驾腰托支撑当前档位 |
| `query_seat_boss_key_status` | 查询老板键开关状态 |
| `query_seat_recline_status` | 查询后排座椅靠背当前角度档位 |

### Tool details

#### `ctrl_seat_driver_memory`

主驾座椅记忆位置调用

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `slot` | `integer` | Yes | 1, 2, 3 | `` | 记忆档位 |

#### `ctrl_seat_heat`

座椅加热档位控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right, rear | `` | 控制位置：主驾/副驾/后排 |
| `level` | `integer` | Yes | 0, 1, 2, 3 | `` | 加热档位，0=关闭 |

#### `ctrl_seat_vent`

座椅通风档位控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right, rear | `` | 控制位置：主驾/副驾/后排 |
| `level` | `integer` | Yes | 0, 1, 2, 3 | `` | 通风档位，0=关闭 |

#### `ctrl_seat_massage`

座椅按摩控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `position` | `string` | Yes | front_left, front_right | `` | 控制位置：主驾/副驾 |
| `mode` | `string` | Yes | off, waist, full_body, pulse | `` | 按摩模式 |
| `level` | `integer` | Yes | 1, 2, 3 | `` | 强度档位 |

#### `ctrl_seat_driver_lumbar`

主驾腰托调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `level` | `integer` | Yes | 1, 2, 3, 4, 5 | `` | 腰托支撑档位 |

#### `ctrl_seat_boss_key`

老板键（副驾座椅前移）

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_seat_rear_recline`

后排座椅靠背角度调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `angle` | `integer` | Yes | 1, 2, 3, 4, 5 | `` | 靠背角度档位，1最直立，5最躺 |

#### `query_seat_heat_status`

查询主驾/副驾/后排座椅加热当前档位

No parameters.

#### `query_seat_vent_status`

查询主驾/副驾/后排座椅通风当前档位

No parameters.

#### `query_seat_massage_status`

查询主驾/副驾按摩当前模式与强度

No parameters.

#### `query_seat_lumbar_status`

查询主驾腰托支撑当前档位

No parameters.

#### `query_seat_boss_key_status`

查询老板键开关状态

No parameters.

#### `query_seat_recline_status`

查询后排座椅靠背当前角度档位

No parameters.
## 空调温控工具集

- **Toolset ID:** `toolset_climate_control`
- **Description:** 空调系统核心温湿度控制与空气净化

| Function | Description |
|---|---|
| `ctrl_ac_power` | 空调系统总电源开关 |
| `ctrl_ac_temperature` | 全车空调温度设置 |
| `ctrl_ac_fan_speed` | 空调风量档位调节 |
| `ctrl_ac_circulation` | 空调内外循环模式切换 |
| `ctrl_ac_vent_mode` | 空调出风口模式 |
| `ctrl_air_purifier` | 车载空气净化器开关 |
| `ctrl_ionizer` | 负离子发生器开关 |
| `query_air_quality` | 车内空气质量查询 |
| `query_ac_power_status` | 查询空调电源开关状态 |
| `query_ac_temperature` | 查询空调当前设定温度 |
| `query_ac_fan_speed` | 查询空调当前风量档位 |
| `query_ac_circulation` | 查询空调当前内外循环模式 |
| `query_ac_vent_mode` | 查询空调当前出风模式 |
| `query_air_purifier_status` | 查询空气净化器开关状态 |
| `query_ionizer_status` | 查询负离子发生器开关状态 |

### Tool details

#### `ctrl_ac_power`

空调系统总电源开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭空调 |

#### `ctrl_ac_temperature`

全车空调温度设置

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `temperature` | `integer` | Yes |  | `` | 设定温度，单位摄氏度 |

#### `ctrl_ac_fan_speed`

空调风量档位调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `level` | `integer` | Yes | 1, 2, 3, 4, 5, 6 | `` | 风量档位 |

#### `ctrl_ac_circulation`

空调内外循环模式切换

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | inner, outer, auto | `` | 循环模式 |

#### `ctrl_ac_vent_mode`

空调出风口模式

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | face, foot, defog, mix | `` | 出风模式 |

#### `ctrl_air_purifier`

车载空气净化器开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_ionizer`

负离子发生器开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `query_air_quality`

车内空气质量查询

No parameters.

#### `query_ac_power_status`

查询空调电源开关状态

No parameters.

#### `query_ac_temperature`

查询空调当前设定温度

No parameters.

#### `query_ac_fan_speed`

查询空调当前风量档位

No parameters.

#### `query_ac_circulation`

查询空调当前内外循环模式

No parameters.

#### `query_ac_vent_mode`

查询空调当前出风模式

No parameters.

#### `query_air_purifier_status`

查询空气净化器开关状态

No parameters.

#### `query_ionizer_status`

查询负离子发生器开关状态

No parameters.
## 座舱舒适配置工具集

- **Toolset ID:** `toolset_cabin_comfort`
- **Description:** 座舱专属舒适配置，包含冰箱、香氛、天幕遮阳等独立功能

| Function | Description |
|---|---|
| `ctrl_fridge` | 车载冷暖冰箱模式控制 |
| `ctrl_fridge_temp` | 冰箱温度调节 |
| `ctrl_aroma_system` | 车载香氛系统 |
| `ctrl_panoramic_shade` | 全景天幕遮阳帘 |
| `ctrl_armrest_heat` | 中央扶手箱加热 |
| `query_fridge_status` | 查询车载冰箱当前工作模式 |
| `query_fridge_temp` | 查询车载冰箱当前设定温度 |
| `query_aroma_status` | 查询香氛系统当前香型/浓度 |
| `query_panoramic_shade_status` | 查询全景天幕遮阳帘当前开度百分比 |
| `query_armrest_heat_status` | 查询中央扶手箱加热开关状态 |

### Tool details

#### `ctrl_fridge`

车载冷暖冰箱模式控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | off, cold, warm | `` | 工作模式：关闭/制冷/制热 |

#### `ctrl_fridge_temp`

冰箱温度调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `temperature` | `integer` | Yes | 3, 5, 7, -6, -12, 40, 50 | `` | 设定温度，单位摄氏度，覆盖冷藏/冷冻/暖箱 |

#### `ctrl_aroma_system`

车载香氛系统

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `scent` | `string` | Yes | tea, flower, wood, ocean | `` | 香氛香型 |
| `intensity` | `string` | Yes | low, medium, high | `` | 浓度强度 |

#### `ctrl_panoramic_shade`

全景天幕遮阳帘

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `openness` | `integer` | Yes | 0, 25, 50, 100 | `` | 遮阳帘开度百分比 |

#### `ctrl_armrest_heat`

中央扶手箱加热

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `query_fridge_status`

查询车载冰箱当前工作模式

No parameters.

#### `query_fridge_temp`

查询车载冰箱当前设定温度

No parameters.

#### `query_aroma_status`

查询香氛系统当前香型/浓度

No parameters.

#### `query_panoramic_shade_status`

查询全景天幕遮阳帘当前开度百分比

No parameters.

#### `query_armrest_heat_status`

查询中央扶手箱加热开关状态

No parameters.
## 影音娱乐工具集

- **Toolset ID:** `toolset_media_entertainment`
- **Description:** 全车影音媒体播放与音量控制

| Function | Description |
|---|---|
| `ctrl_music_play` | 播放指定类型音乐/歌单 |
| `ctrl_music_next` | 切换下一首曲目 |
| `ctrl_music_prev` | 切换上一首曲目 |
| `ctrl_volume_media` | 媒体音量档位调节 |
| `ctrl_sound_mode` | 音响音效模式 |
| `ctrl_radio_tune` | 电台频率切换 |
| `ctrl_media_mute` | 全局媒体静音开关 |
| `ctrl_rear_entertainment` | 后排娱乐系统开关 |
| `query_music_status` | 查询当前播放音乐类型及曲目序号 |
| `query_volume_media` | 查询媒体音量当前档位及静音状态 |
| `query_sound_mode_status` | 查询音响当前音效模式 |
| `query_radio_status` | 查询收音机当前锁定频率 |
| `query_rear_entertainment_status` | 查询后排娱乐系统开关状态 |

### Tool details

#### `ctrl_music_play`

播放指定类型音乐/歌单

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `category` | `string` | Yes | light, pop, classic, radio, favorite | `` | 音乐类型 |

#### `ctrl_music_next`

切换下一首曲目

No parameters.

#### `ctrl_music_prev`

切换上一首曲目

No parameters.

#### `ctrl_volume_media`

媒体音量档位调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `level` | `integer` | Yes |  | `` | 音量档位 |

#### `ctrl_sound_mode`

音响音效模式

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | hifi, vocal, theater, bass_boost | `` | 音效模式 |

#### `ctrl_radio_tune`

电台频率切换

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `frequency` | `string` | Yes | FM89.3, FM91.5, FM97.4, FM103.9, AM639, AM1008 | `` | 电台频率 |

#### `ctrl_media_mute`

全局媒体静音开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭静音 |

#### `ctrl_rear_entertainment`

后排娱乐系统开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `query_music_status`

查询当前播放音乐类型及曲目序号

No parameters.

#### `query_volume_media`

查询媒体音量当前档位及静音状态

No parameters.

#### `query_sound_mode_status`

查询音响当前音效模式

No parameters.

#### `query_radio_status`

查询收音机当前锁定频率

No parameters.

#### `query_rear_entertainment_status`

查询后排娱乐系统开关状态

No parameters.
## 导航出行工具集

- **Toolset ID:** `toolset_navigation`
- **Description:** 导航路径规划、POI搜索、路况与沿途服务查询

| Function | Description |
|---|---|
| `ctrl_nav_start` | 发起导航到指定目的地 |
| `ctrl_nav_stop` | 结束当前导航 |
| `ctrl_nav_route_pref` | 导航路线偏好设置 |
| `query_poi_nearby` | 周边POI兴趣点搜索 |
| `query_traffic_status` | 当前行驶路线路况查询 |
| `query_charging_station` | 沿途充电站推荐查询 |
| `query_route_charge_plan` | 沿途充电规划 |
| `query_nav_status` | 查询当前导航是否激活及目的地名称 |

### Tool details

#### `ctrl_nav_start`

发起导航到指定目的地

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `destination` | `string` | Yes |  | `` | 目的地名称或地址 |

#### `ctrl_nav_stop`

结束当前导航

No parameters.

#### `ctrl_nav_route_pref`

导航路线偏好设置

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | fastest, shortest, no_highway, no_toll | `` | 路线偏好 |

#### `query_poi_nearby`

周边POI兴趣点搜索

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `keyword` | `string` | Yes |  | `` | 搜索关键词 |
| `type` | `string` | Yes | gas, charging, restaurant, parking | `` | POI类型 |

#### `query_traffic_status`

当前行驶路线路况查询

No parameters.

#### `query_charging_station`

沿途充电站推荐查询

No parameters.

#### `query_route_charge_plan`

沿途充电规划

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `soc_threshold` | `integer` | Yes |  | `` | 剩余电量阈值，单位% |

#### `query_nav_status`

查询当前导航是否激活及目的地名称

No parameters.
## 车辆信息工具集

- **Toolset ID:** `toolset_vehicle_info`
- **Description:** 车辆内外各类只读状态信息查询

| Function | Description |
|---|---|
| `query_battery_status` | 动力电池状态查询 |
| `query_range_mileage` | 查询剩余续航里程 |
| `query_energy_consumption` | 能耗统计查询 |
| `query_tire_pressure` | 查询四轮胎压状态 |

### Tool details

#### `query_battery_status`

动力电池状态查询

No parameters.

#### `query_range_mileage`

查询剩余续航里程

No parameters.

#### `query_energy_consumption`

能耗统计查询

No parameters.

#### `query_tire_pressure`

查询四轮胎压状态

No parameters.
## 充电管理工具集

- **Toolset ID:** `toolset_charging_management`
- **Description:** 新能源车辆端充电控制与管理

| Function | Description |
|---|---|
| `ctrl_charge_start` | 开始交流/直流充电 |
| `ctrl_charge_stop` | 停止充电 |
| `ctrl_charge_limit` | 充电上限设置 |
| `ctrl_charge_schedule` | 预约充电设置 |
| `ctrl_v2l_discharge` | VTOL对外放电开关 |
| `query_charge_status` | 充电状态查询 |

### Tool details

#### `ctrl_charge_start`

开始交流/直流充电

No parameters.

#### `ctrl_charge_stop`

停止充电

No parameters.

#### `ctrl_charge_limit`

充电上限设置

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `percent` | `integer` | Yes | 50, 60, 70, 80, 90, 100 | `` | 充电上限百分比 |

#### `ctrl_charge_schedule`

预约充电设置

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `start_time` | `string` | Yes |  | `` | 开始时间，格式HH:MM |
| `end_time` | `string` | Yes |  | `` | 结束时间，格式HH:MM |

#### `ctrl_v2l_discharge`

VTOL对外放电开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭放电 |

#### `query_charge_status`

充电状态查询

No parameters.
## 通讯通话工具集

- **Toolset ID:** `toolset_communication`
- **Description:** 车载电话、短信、通讯录管理

| Function | Description |
|---|---|
| `ctrl_phone_call` | 拨打指定联系人/号码电话 |
| `ctrl_phone_answer` | 接听当前来电 |
| `ctrl_phone_hangup` | 挂断当前通话 |
| `query_contact_search` | 通讯录匹配联系人查询 |
| `ctrl_message_read` | 播报未读短信/消息 |

### Tool details

#### `ctrl_phone_call`

拨打指定联系人/号码电话

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `contact` | `string` | Yes |  | `` | 联系人名或电话号码 |

#### `ctrl_phone_answer`

接听当前来电

No parameters.

#### `ctrl_phone_hangup`

挂断当前通话

No parameters.

#### `query_contact_search`

通讯录匹配联系人查询

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `keyword` | `string` | Yes |  | `` | 姓名或号码关键词 |

#### `ctrl_message_read`

播报未读短信/消息

No parameters.
## 驾驶辅助工具集

- **Toolset ID:** `toolset_driving_assist`
- **Description:** ADAS驾驶辅助、泊车控制、显示设置

| Function | Description |
|---|---|
| `ctrl_cruise_control` | 定速巡航开关与设置 |
| `ctrl_lane_keep` | 车道保持辅助开关 |
| `ctrl_acc_distance` | 自适应巡航跟车距离调节 |
| `ctrl_energy_recovery` | 能量回收强度调节 |
| `ctrl_auto_park` | 自动泊车启动 |
| `ctrl_remote_park` | 遥控泊车控制 |
| `ctrl_hud_display` | HUD抬头显示开关 |
| `query_cruise_control_status` | 查询定速巡航开关状态及设定车速 |
| `query_lane_keep_status` | 查询车道保持辅助开关状态 |
| `query_acc_distance_status` | 查询自适应巡航当前跟车距离档位 |
| `query_energy_recovery_status` | 查询能量回收当前强度模式 |
| `query_hud_status` | 查询HUD抬头显示开关状态 |

### Tool details

#### `ctrl_cruise_control`

定速巡航开关与设置

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |
| `speed` | `integer` | No |  | `` | 巡航车速，单位km/h，可选 |

#### `ctrl_lane_keep`

车道保持辅助开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `ctrl_acc_distance`

自适应巡航跟车距离调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `level` | `integer` | Yes | 1, 2, 3, 4 | `` | 跟车距离档位，1=最近 |

#### `ctrl_energy_recovery`

能量回收强度调节

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `level` | `string` | Yes | low, medium, high, one_pedal | `` | 回收强度模式 |

#### `ctrl_auto_park`

自动泊车启动

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `mode` | `string` | Yes | vertical, parallel | `` | 车位类型：垂直/平行 |

#### `ctrl_remote_park`

遥控泊车控制

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `direction` | `string` | Yes | forward, backward | `` | 行驶方向 |

#### `ctrl_hud_display`

HUD抬头显示开关

| Parameter | Type | Required | Enum | Default | Description |
|---|---|:---:|---|---|---|
| `status` | `string` | Yes | on, off | `` | 开启/关闭 |

#### `query_cruise_control_status`

查询定速巡航开关状态及设定车速

No parameters.

#### `query_lane_keep_status`

查询车道保持辅助开关状态

No parameters.

#### `query_acc_distance_status`

查询自适应巡航当前跟车距离档位

No parameters.

#### `query_energy_recovery_status`

查询能量回收当前强度模式

No parameters.

#### `query_hud_status`

查询HUD抬头显示开关状态

No parameters.
## 座舱模式工具集

- **Toolset ID:** `toolset_cabin_modes`
- **Description:** 高频场景一键触发，内部调用各工具集原子工具执行

| Function | Description |
|---|---|
| `trigger_car_wash_mode` | 启动洗车模式 |
| `trigger_rest_mode` | 启动休息模式 |
| `trigger_commute_mode` | 启动通勤模式 |
| `trigger_child_mode` | 启动儿童模式 |
| `trigger_charge_mode` | 充电节能模式 |
| `trigger_camp_mode` | 露营模式 |
| `trigger_pet_mode` | 宠物模式 |
| `trigger_stealth_mode` | 隐私模式 |

### Tool details

#### `trigger_car_wash_mode`

启动洗车模式

No parameters.

#### `trigger_rest_mode`

启动休息模式

No parameters.

#### `trigger_commute_mode`

启动通勤模式

No parameters.

#### `trigger_child_mode`

启动儿童模式

No parameters.

#### `trigger_charge_mode`

充电节能模式

No parameters.

#### `trigger_camp_mode`

露营模式

No parameters.

#### `trigger_pet_mode`

宠物模式

No parameters.

#### `trigger_stealth_mode`

隐私模式

No parameters.