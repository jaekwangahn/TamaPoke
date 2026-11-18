#include "i18n.h"
#include "pet.h"        // MED_COUNT
#include <Preferences.h>

Lang gLang = LANG_DEFAULT;

// Tabla de cadenas [idioma][id]. Sin acentos ni enes: la fuente bitmap del
// firmware no los tiene (por eso el espanol ya iba "Esta", "bano", etc.).
static const char *const STRINGS[LANG_COUNT][STR_COUNT] = {
  // ---------------- EN ----------------
  {
    "Evolving!", "Yum yum!", "It likes it!", "It's hungry!", "Needs a bath!",
    "Worn out...", "Feeling sad...", "A bit chubby...", "It's SHINY!!", "It's happy",
    "THANKS! Farewell", "It ran away...", "Bye! Waving goodbye...",
    "EGG", "Legendary egg!?", "Rare egg!", "Tap the egg...", "It moves!", "Almost there!",
    "POKEDEX %u/1025",
    "%s%s Lv.%u",
    "Release %s?", "YES", "NO",
    "SETTINGS", "swipe up: back", "Lang",
    "MEDAL!", "AWESOME!", "%u DAY STREAK!",
    "STREAK %u  best %u", "BOND",
    "AGE %lu days", "tap name: rename",
    "ABILITY", "STA", "RES", "VIT", "WGT",
    "MEDALS %d/%d", "tap: back",
    "NAME:", "tap: back",
    "FOOD", "JOY", "ENE", "HYG",
    "PROGRESS", "Lv.%u", "%u min to Lv.%u", "EVOLUTION", "Final form",
    "Ready to evolve!", "All needs >=40 to evolve",
    "Evolves in %ud %uh", "Slip-ups: %u",
    "SND ON", "SND OFF",
    "EVOLVE!", "%s wants to tell you...", "%s feels abandoned...",
    "Evolve?", "Keep form", "Say goodbye?", "Goodbye", "Stay together",
    "Choose your starter",
    "No sprites", "Load them onto the SD",

    "Feeling sick...",

    "Collect", "Release", "FARM %u", "Farm is empty", "Swap in", "Hatch the egg first",
    "Off to the farm!",

    "Needs an SD card",

    "KANTO", "JOHTO", "HOENN", "SINNOH", "UNOVA", "KALOS", "ALOLA", "GALAR", "PALDEA",

    "SHOP  %ld", "BAG",
    "CANDY", "MEDICINE", "STA POTION", "VIT POTION", "CHARM",
    "EGG", "RARE EGG", "LEGEND EGG",
    "Bought!", "Not enough points", "None left - shop", "Done!", "Already at max", "Not sick",
    "Used on next egg",
    "Only with no partner", "No partner", "Pick an egg", "SHOP",

    "%u games %u wins", "+%ld points", "New record!",

    "WI-FI", "Phone: join TamaPoke", "open 192.168.4.1", "Wi-Fi saved", "Syncing chess...",
    "Sync failed",
    "Happy New Year!", "Happy Lunar New Year!", "Lantern Festival!", "Happy Valentine's!",
    "Happy Easter!", "Happy Mother's Day!", "Dragon Boat Festival!", "Happy Father's Day!",
    "Happy Canada Day!", "Mid-Autumn Festival!", "Happy Thanksgiving!", "Happy Halloween!",
    "Merry Christmas!", "Happy Birthday %s!",
    "%s DEX DONE!", "+%d pts  Shiny egg!", "SHINY LEGEND",
    "Loves the rain!", "Hates the rain...", "Snow! Yay!", "Brr... so cold", "Lovely sunshine!",
    "STEPS TODAY %lu",
    "ALBUM", "ALBUM %d", "No memories yet",
    "Farewell", "Ran away", "Released", "To the farm",
    "Easygoing", "Glutton", "Playful", "Tidy", "Lazy",
    "Good morning!", "Getting sleepy...", "I want a berry...", "Bath time?", "Play with me!",
    "So tired...", "You won %u games!", "Good games yesterday!", "I love you, %s!", "Hi!",
    "My tummy rumbles", "Let's play!", "I like being clean", "Five more minutes", "What a nice day",
    "Bow", "Flower", "Cap", "Witch hat", "Top hat", "Crown",
    "ACCESSORIES", "OWNED", "WEARING", "Put on!", "Taken off",
    "%s came to visit!",
    "No backup yet", "Restore this backup?", "Restoring...",
    "So comfy~", "FARM HARVEST!", "+%u pts",
    "Hehe~", "Giggle giggle!", "Haha, that tickles!", "Hey, stop it!", "Wheee!",
    "Yummy!", "So fun!", "*big stretch*", "Took my medicine!",
    "EVO POTION", "Not needed now",
    "LOCKED", "Register every egg Pokemon of", "NEW REGION!",
    "energy drains %u%% slower", "joy drains %u%% slower", "gets sick %u%% less",
    "%u/%u", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",
    "STATS", "Raised", "Pokedex", "Shiny", "Regions open", "Regions done", "Best streak", "Medals",
    "Steps", "Chess points", "Farm",
    "tap: open",
    "SYNC NOW", "No new games",
    "also your chess.com name", "Set chess.com name in WI-FI",
    "10,000 steps! +10 pts",
    "ACTIVITIES", "TOURNAMENT", "Win a Mega Stone", "Asleep...", "swipe right: back",
    "MEGA STONE", "MEGA %s%s", "No Mega form", "Already Mega", "MEGA EVOLVED!", "Back to normal",
    "100m DASH", "HURDLES", "50m SWIM", "LONG JUMP", "HIGH JUMP",
    "Speed counts", "Speed and spring", "Speed and stamina", "Power and speed", "Power counts",
    "tap: start", "GO!", "GOLD!", "SILVER", "BRONZE",
    "Got a Mega Stone!", "Today's stone is won", "%u.%u s", "%u.%02u m", "vs %s & %s", "Too tired", "YOU",
    "Bonus: Electric, Fighting", "Bonus: Fighting, Bug", "Bonus: Water, Ice", "Bonus: Fighting, Flying",
    "Bonus: Flying, Psychic", "You +%u%%", "You -%u%%",
    "In the grass: ",
    "Phone steps code",
    "WEIGHTS", "CLIMBING", "DIVING", "SKATING",
    "TORCH RUN", "TRAIL RUN", "Power counts", "Power and agility",
    "Technique counts", "Speed counts", "Stamina counts", "A bit of everything",
    "Bonus: Fighting, Steel, Rock", "Bonus: Rock, Ground, Bug", "Bonus: Water, Psychic, Flying", "Bonus: Ice, Fairy",
    "Bonus: Fire, Dragon", "Bonus: Dark, Ghost, Poison", "%u kg", "%u.%u pts",
    "%u:%02u", "GYMS", "%s GYMS", "Badges %u/8",
    "LEAGUE", "CHAMPION", "swipe: regions", "Win all 8 badges first",
    "Already tried today", "Win the badge before", "Need 2 in the farm", "Elite Four",
    "Champion", "CHALLENGE", "REMATCH", "PICK 2",
    "NEXT", "LINEUP", "Tap yours to swap", "START",
    "%s  Round %u/3", "You win!", "You lose", "tap: continue",
    "Got the %s!", "Also: %s", "Lost. Try again tomorrow", "CHAMPION!",
    "League over. Try tomorrow", "Normal", "Fire", "Water",
    "Grass", "Electric", "Ice", "Fighting",
    "Poison", "Ground", "Flying", "Psychic",
    "Bug", "Rock", "Ghost", "Dragon",
    "Dark", "Steel", "Fairy",
    "%u badges",
    "FUN", "Fish, grow, dig...", "FUN  %ld", "ORCHARD",
    "FISHING", "MINING", "TREASURE", "Grass Bug Ground Poison Fairy",
    "Needs Water Ice Dragon", "Needs Rock Steel Fighting Fire", "Normal Fly Elec Psy Ghost Dark", "%u can go",
    "%u pts", "Won at chess: double luck!", "Played chess: +1 loot", "Play chess for good luck",
    "WHO GOES?", "Out treasure hunting", "%u min left", "%u h left",
    "%u ready", "Back! Collect", "PLANT", "WATER",
    "HARVEST", "Watered", "Ripe!", "Clock not set yet",
    "CAST", "Wait for a bite...", "Now! Tap!", "Great catch!",
    "Just a small one", "DIG", "Tap the rock!", "SET OFF",
    "Back in 1 hour", "COLLECT", "Someone is already out", "You got",
    "STA FRUIT", "RES FRUIT", "VIT FRUIT", "GOLD FRUIT",
    "FISH", "PEARL", "MEGA SHARD", "WATER EGG",
    "FOSSIL EGG", "Full today: just a snack", "10 shards = 1 Mega Stone", "STA +3",
    "RES +3", "VIT +3", "All +3, so happy!", "So happy!",
    "Yum, so full!",
    "FARM", "%u Pokemon", "RESTORE", "x%u",
    "Good at: %s, %s", "HP %u  ATK %u  DEF %u  SPD %u", "STA %u  RES %u  VIT %u", "Try other teammates,",
    "or train with fruits", "Badges", "League titles", "Fun finds",
    "%u h", "%u min",
    "Buy for %u pts?", "Spend %u pts?",
    "SFX ON", "SFX OFF", "BGM ON", "BGM OFF",
    "Study", "Deepgong", "End study", "Study complete", "Choose a duration", "Could not save", "End study?",
    "Vol", "Level Potion",
  },
  // ---------------- ZH ----------------
  // Chino simplificado. Los especificadores van en el orden de la fila
  // inglesa: T(id) se rellena con varargs en un orden fijo en cada llamada. Todo debe estar en GB2312, que es lo que
  // trae la fuente (lo comprueba test/test_tools.py).
  {
    "正在进化！", "好吃！", "它很喜欢！", "肚子饿了！", "该洗澡了！",
    "累坏了...", "好难过...", "有点胖了...", "是异色！！", "心情很好",
    "谢谢！再见", "它离家出走了...", "拜拜！挥手告别...",

    "蛋", "传说之蛋！？", "稀有的蛋！", "摸摸蛋...", "动了！", "快孵出来了！",

    "图鉴 %u/1025",

    "%s%s Lv.%u",

    "放生%s？", "是", "否",

    "设置", "上滑：返回", "语言",

    "奖章！", "太棒了！", "连续 %u 天！",

    "连续 %u  最高 %u", "羁绊",

    "年龄 %lu 天", "点名字：改名",

    "能力", "体力", "抵抗力", "活力", "体重",

    "奖章 %d/%d", "点一下：返回",

    "名字：", "点一下：返回",

    "饱食", "心情", "精力", "清洁",

    "成长", "Lv.%u", "%u分钟后 Lv.%u", "进化", "最终形态",

    "可以进化了！", "全部40以上才能进化",

    "%u天%u小时后可进化", "照顾失误：%u",

    "声音开", "声音关",

    "进化！", "%s有话想说...", "%s觉得被冷落了...",

    "要进化吗？", "保持原样", "要告别吗？", "再见", "继续在一起",

    "选择你的初始宝可梦",

    "没有图片", "请放进SD卡",

    "生病了...",

    "收藏", "放生", "农场 %u", "农场还是空的", "换出来", "先把蛋孵出来",
    "去农场啦！",

    "需要SD卡",

    "关都", "城都", "丰缘", "神奥", "合众", "卡洛斯", "阿罗拉", "伽勒尔", "帕底亚",

    "商店  %ld分", "背包",
    "糖果", "药", "体力药剂", "活力药剂", "异色符",
    "普通蛋", "稀有蛋", "传说蛋",
    "买好了！", "积分不够", "没有了，去商店买", "用好了！", "已经满了", "没有生病",
    "下一颗蛋自动生效",
    "没有伙伴时才能孵", "没有伙伴", "选一颗蛋来孵", "去商店",

    "%u盘 赢%u盘", "+%ld分", "创纪录了！",

    "Wi-Fi", "手机连 TamaPoke", "打开 192.168.4.1", "Wi-Fi 已保存", "正在同步棋局...",
    "同步失败",

    "新年快乐！", "春节快乐！", "元宵节快乐！", "情人节快乐！",
    "复活节快乐！", "母亲节快乐！", "端午安康！", "父亲节快乐！",
    "加拿大国庆日快乐！", "中秋节快乐！", "感恩节快乐！", "万圣节快乐！",
    "圣诞快乐！", "%s 生日快乐！",

    "%s图鉴集齐！", "+%d分 送异色传说蛋", "异色传说蛋",

    "下雨真开心！", "讨厌下雨…", "下雪啦，好开心！", "好冷…", "晒太阳真舒服！",

    "今日步数 %lu",

    "回忆录", "回忆录 %d", "还没有回忆",

    "告别", "离家出走", "放生", "进了农场",

    "性格：乖巧", "性格：贪吃", "性格：爱玩", "性格：爱干净", "性格：懒洋洋",

    "早上好！", "有点困了…", "想吃树果…", "想洗澡…", "陪我玩嘛！",
    "好累呀…", "昨天赢了%u盘，好厉害！", "昨天下棋辛苦啦！", "最喜欢 %s 了！", "嗨！",
    "肚子咕咕叫…", "我们来玩吧！", "我爱干干净净的", "再躺一会儿…", "今天也很开心",

    "蝴蝶结", "小花", "棒球帽", "巫师帽", "礼帽", "皇冠",
    "饰品", "已拥有", "戴着", "戴上了", "摘下了",

    "%s来串门啦！",

    "还没有备份", "用这份备份覆盖现在的进度？", "正在恢复…",

    "好舒服～", "农场收获！", "+%u分",

    "嘿嘿～", "咯咯咯！", "哈哈哈，好痒！", "别闹啦！", "转圈圈～",

    "好吃！", "好开心！", "伸个懒腰～", "自己吃了药！",

    "进化药水", "现在用不上",

    "未解锁", "集齐上一地区蛋里能孵的宝可梦", "新地区解锁！",

    "精力消耗慢 %u%%", "心情消耗慢 %u%%", "生病几率低 %u%%",

    "%u月%u日", "周日", "周一", "周二", "周三", "周四", "周五", "周六",

    "统计", "养过的宝可梦", "图鉴登记", "异色", "已解锁地区", "集齐地区", "最长连续照顾", "获得奖章",
    "总步数", "下棋赚的积分", "农场",
    "点一下：查看",
    "立即同步", "没有新对局",
    "还能设置 chess.com 名字", "先在 Wi-Fi 里设置 chess.com 名字",
    "走满一万步！+10 分",
    "活动", "锦标赛", "赢了得Mega石", "在睡觉...", "右滑：返回",
    "Mega石", "超级%s%s", "它没有超级进化", "已经是超级形态", "超级进化！", "变回来了",
    "100米短跑", "跨栏", "50米游泳", "跳远", "跳高",
    "比速度", "比速度和弹跳", "比速度和耐力", "比力量和速度", "比力量",
    "点一下：开始", "开始！", "金牌！", "银牌", "铜牌",
    "获得Mega石！", "今天的Mega石已拿过", "%u.%u 秒", "%u.%02u 米", "对手：%s、%s", "太累了", "你",
    "电、格斗属性有加成", "格斗、虫属性有加成", "水、冰属性有加成", "格斗、飞行属性有加成",
    "飞行、超能力属性有加成", "你 +%u%%", "你 -%u%%",
    "草丛里有：",
    "手机步数配对码",
    "举重", "攀岩", "跳水", "速度滑冰",
    "火炬长跑", "越野障碍", "比力量", "比力量和敏捷",
    "比技巧", "比速度", "比耐力", "什么都比一点",
    "格斗、钢、岩石属性有加成", "岩石、地面、虫属性有加成", "水、超能力、飞行属性有加成", "冰、妖精属性有加成",
    "火、龙属性有加成", "恶、幽灵、毒属性有加成", "%u 公斤", "%u.%u 分",
    "%u:%02u", "道馆", "%s道馆", "徽章 %u/8",
    "联盟大赛", "冠军", "左右滑：换地区", "先集齐 8 枚徽章",
    "今天已经挑战过了", "先拿到上一枚徽章", "农场里要有 2 只宝可梦", "四天王",
    "冠军", "挑战", "再挑战", "选两只队友",
    "下一步", "排阵容", "点自己的宝可梦换人", "开始",
    "%s  第 %u/3 局", "赢了！", "输了", "点一下：继续",
    "获得%s！", "还得到：%s", "这次输了，明天再来", "你是冠军！",
    "联盟挑战结束，明天再来", "一般属性", "火属性", "水属性",
    "草属性", "电属性", "冰属性", "格斗属性",
    "毒属性", "地面属性", "飞行属性", "超能力属性",
    "虫属性", "岩石属性", "幽灵属性", "龙属性",
    "恶属性", "钢属性", "妖精属性",
    "已有 %u 枚徽章",
    "趣玩", "钓鱼、种果子、挖宝……", "趣玩  %ld分", "果园",
    "钓鱼", "挖矿", "寻宝", "要草、虫、地面、毒、妖精属性",
    "要水、冰、龙属性", "要岩石、钢、格斗、火属性", "要一般、飞行、电、超能力、幽灵、恶属性", "%u 只能去",
    "%u 分", "下棋赢了：好运翻倍！", "下了棋：收获多 1 个", "下盘棋会带来好运",
    "派谁去？", "出门寻宝去了", "还要 %u 分钟", "还要 %u 小时",
    "%u 块熟了", "回来了，快领取", "种下", "浇水",
    "收获", "浇过水了", "熟了！", "时间还没同步",
    "抛竿", "等鱼上钩……", "快点屏幕收竿！", "钓得好！",
    "只钓到一条小鱼", "开挖", "点石头把它敲开！", "出发",
    "1 小时后回来", "领取", "已经有宝可梦出门了", "收获",
    "体力果", "抵抗果", "活力果", "金果",
    "鱼", "珍珠", "Mega石碎片", "水属性蛋",
    "化石蛋", "今天吃够了，只填饱肚子", "集齐 10 片自动变成 Mega石", "体力 +3",
    "抵抗力 +3", "活力 +3", "三项都 +3，好开心！", "好开心！",
    "好吃，吃得好饱！",
    "农场", "%u 只宝可梦", "恢复备份", "有 %u",
    "擅长：%s、%s", "HP %u  攻 %u  防 %u  速 %u", "体力 %u  抵抗力 %u  活力 %u", "试试换别的队友，",
    "或者吃果子练训练值", "徽章总数", "联盟冠军", "趣玩收获",
    "%u 小时", "%u 分钟",
    "花 %u 分购买？", "花 %u 分？",
    "音效开", "音效关", "音乐开", "音乐关",
    "计时", "迪普", "结束计时", "时间到", "选择时长", "保存失败", "结束计时?",
    "音量", "升级药水",
  },
  // ---------------- KO ----------------
  // Korean UTF-8; keep printf arguments in the same order as English.
  {
    "진화 중!", "냠냠!", "마음에 들어!", "배고파!", "씻고 싶어!",
    "기진맥진...", "기분이 울적해...", "조금 통통해...", "색이 다른 모습!!", "행복해",
    "고마워! 안녕", "집을 떠났어...", "안녕! 또 만나!",
    "알", "전설의 알!?", "희귀한 알!", "알을 만져 봐...", "움직여!", "곧 태어나!",
    "도감 %u/1025",
    "%s%s Lv.%u",
    "%s, 놓아줄까?", "예", "아니요",
    "설정", "위로 밀어 돌아가기", "언어",
    "메달 획득!", "대단해!", "%u일 연속!",
    "연속 %u  최고 %u", "유대",
    "나이 %lu일", "이름을 눌러 변경",
    "능력", "체력", "저항", "활력", "체중",
    "메달 %d/%d", "눌러 돌아가기",
    "이름:", "눌러 돌아가기",
    "포만", "기분", "에너지", "청결",
    "성장", "Lv.%u", "%u분 후 Lv.%u", "진화", "최종 진화형",
    "진화할 수 있어!", "모든 상태 40 이상 필요",
    "진화까지 %u일 %u시간", "돌봄 실수: %u",
    "소리 켬", "소리 끔",
    "진화!", "%s, 할 말이 있대...", "%s, 외로운가 봐...",
    "진화할까?", "지금 모습 유지", "작별할까?", "잘 가", "함께 지내기",
    "첫 동료를 골라줘",
    "이미지 없음", "SD 카드에 넣어 주세요",

    "몸이 아파...",

    "데려오기", "놓아주기", "농장 %u", "농장이 비었어", "교대", "먼저 알을 부화시켜 줘",
    "농장으로 갈게!",

    "SD 카드가 필요해",

    "관동", "성도", "호연", "신오", "하나", "칼로스", "알로라", "가라르", "팔데아",

    "상점  %ld", "가방",
    "사탕", "약", "체력 물약", "활력 물약", "빛나는부적",
    "알", "희귀한 알", "전설의 알",
    "구매 완료!", "포인트가 부족해", "없어, 상점에 가 봐", "사용 완료!", "이미 최대야", "아프지 않아",
    "다음 알에 자동 사용",
    "동료가 없을 때만", "동료 없음", "알을 골라줘", "상점",

    "%u판 %u승", "+%ld포인트", "새 기록!",

    "Wi-Fi", "휴대폰: TamaPoke 연결", "192.168.4.1 열기", "Wi-Fi 저장 완료", "체스 동기화 중...",
    "동기화 실패",
    "새해 복 많이 받아!", "즐거운 설날!", "정월 대보름!", "즐거운 밸런타인데이!",
    "즐거운 부활절!", "어머니의 날!", "즐거운 단오!", "아버지의 날!",
    "캐나다의 날!", "즐거운 추석!", "즐거운 추수감사절!", "즐거운 핼러윈!",
    "메리 크리스마스!", "%s, 생일 축하해!",
    "%s 도감 완성!", "+%d포인트  색이 다른 알!", "색이 다른 전설의 알",
    "비가 좋아!", "비는 싫어...", "눈이다! 신나!", "으, 추워", "햇살이 좋아!",
    "오늘 걸음 %lu",
    "추억 앨범", "앨범 %d", "아직 추억이 없어",
    "작별", "가출", "놓아줌", "농장으로",
    "느긋함", "먹보", "장난꾸러기", "깔끔함", "게으름",
    "좋은 아침!", "슬슬 졸려...", "나무열매 먹고 싶어...", "목욕할까?", "같이 놀자!",
    "너무 피곤해...", "%u판이나 이겼네!", "어제 체스 수고했어!", "사랑해, %s!", "안녕!",
    "배에서 꼬르륵", "우리 놀자!", "깨끗한 게 좋아", "5분만 더...", "기분 좋은 날이야",
    "리본", "꽃", "모자", "마녀 모자", "실크해트", "왕관",
    "꾸미기", "보유", "착용 중", "착용했어!", "벗었어",
    "%s, 놀러 왔어!",
    "백업이 없어", "이 백업을 복원할까?", "복원 중...",
    "정말 편안해~", "농장 수확!", "+%u포인트",
    "헤헤~", "까르르!", "하하, 간지러워!", "이제 그만!", "빙글빙글!",
    "맛있어!", "정말 재밌어!", "쭈욱 기지개", "약을 먹었어!",
    "진화 물약", "지금은 필요 없어",
    "잠김", "이전 지역의 알 포켓몬 등록", "새 지역 개방!",
    "에너지 소모 %u%% 감소", "기분 소모 %u%% 감소", "질병 확률 %u%% 감소",
    "%u월 %u일", "일", "월", "화", "수", "목", "금", "토",
    "통계", "키운 동료", "도감", "색이 다른 모습", "열린 지역", "완성한 지역", "최고 연속", "메달",
    "걸음", "체스 포인트", "농장",
    "눌러 열기",
    "지금 동기화", "새 대국 없음",
    "chess.com 이름도 입력", "Wi-Fi에서 체스 이름 입력",
    "1만 걸음! +10포인트",
    "활동", "스포츠 대회", "메가스톤에 도전!", "자는 중...", "오른쪽으로 밀어 복귀",
    "메가스톤", "메가%s%s", "메가진화형 없음", "이미 메가진화 중", "메가진화!", "원래 모습으로",
    "100m 달리기", "허들", "50m 수영", "멀리뛰기", "높이뛰기",
    "속도가 중요해", "속도와 탄력", "속도와 지구력", "힘과 속도", "힘이 중요해",
    "눌러 시작", "출발!", "금메달!", "은메달", "동메달",
    "메가스톤 획득!", "오늘은 이미 받았어", "%u.%u초", "%u.%02um", "상대: %s, %s", "너무 피곤해", "내 동료",
    "전기·격투 유리", "격투·벌레 유리", "물·얼음 유리", "격투·비행 유리",
    "비행·에스퍼 유리", "내 보너스 +%u%%", "내 보너스 -%u%%",
    "풀숲에서: ",
    "휴대폰 걸음 연동 코드",
    "역도", "암벽등반", "다이빙", "스케이팅",
    "성화 달리기", "산길 달리기", "힘이 중요해", "힘과 민첩성",
    "기술이 중요해", "속도가 중요해", "지구력이 중요해", "고른 능력이 중요해",
    "격투·강철·바위 유리", "바위·땅·벌레 유리", "물·에스퍼·비행 유리", "얼음·페어리 유리",
    "불꽃·드래곤 유리", "악·고스트·독 유리", "%ukg", "%u.%u점",
    "%u:%02u", "체육관", "%s 체육관", "배지 %u/8",
    "리그", "챔피언", "밀어서 지역 변경", "배지 8개를 모아줘",
    "오늘은 이미 도전했어", "이전 배지가 필요해", "농장에 두 마리 필요", "사천왕",
    "챔피언", "도전", "재도전", "두 마리 선택",
    "다음", "팀 편성", "내 동료를 눌러 교체", "시작",
    "%s  %u/3경기", "이겼어!", "졌어", "눌러 계속",
    "%s 획득!", "추가 보상: %s", "내일 다시 도전하자", "챔피언!",
    "리그 종료, 내일 다시", "노말", "불꽃", "물",
    "풀", "전기", "얼음", "격투",
    "독", "땅", "비행", "에스퍼",
    "벌레", "바위", "고스트", "드래곤",
    "악", "강철", "페어리",
    "배지 %u개",
    "즐길 거리", "낚시, 재배, 채굴...", "활동  %ld", "과수원",
    "낚시", "채굴", "보물찾기", "풀·벌레·땅·독·페어리 필요",
    "물·얼음·드래곤 필요", "바위·강철·격투·불꽃 필요", "노말·비행·전기·에스퍼·고스트·악", "%u마리 참가 가능",
    "%u포인트", "체스 승리: 행운 두 배!", "체스 참가: 보상 +1", "체스를 두면 행운이 와",
    "누가 갈까?", "보물찾기 떠났어", "%u분 남음", "%u시간 남음",
    "%u곳 수확 가능", "돌아왔어! 받아줘", "심기", "물 주기",
    "수확", "물 줬어", "다 익었어!", "먼저 시간을 맞춰줘",
    "던지기", "입질을 기다려...", "지금! 눌러!", "잘 낚았어!",
    "작은 물고기야", "채굴", "바위를 눌러 깨 줘!", "출발",
    "1시간 후 돌아와", "받기", "이미 누군가 외출 중", "획득",
    "체력 열매", "저항 열매", "활력 열매", "황금 열매",
    "물고기", "진주", "메가스톤 조각", "물타입 알",
    "화석 알", "오늘은 간식으로만", "조각 10개면 메가스톤", "체력 +3",
    "저항 +3", "활력 +3", "모두 +3, 기분 최고!", "기분 최고!",
    "맛있어, 배불러!",
    "농장", "포켓몬 %u마리", "백업 복원", "%u개",
    "특기: %s, %s", "HP %u  공 %u  방 %u  속 %u", "체력 %u  저항 %u  활력 %u", "다른 동료를 골라 봐",
    "열매로 훈련해도 좋아", "배지", "리그 우승", "활동 수확",
    "%u시간", "%u분",
    "%u포인트에 살까?", "%u포인트를 쓸까?",
    "효과음 켬", "효과음 끔", "BGM 켬", "BGM 끔",
    "공부하기", "딥공이", "공부 종료", "공부 완료", "시간 선택", "저장 실패", "공부 종료?",
    "소리", "레벨 물약",
  },
};

// Nombres de medalla en sus tres longitudes [idioma][medalla].
static const char *const MED_NAME[LANG_COUNT][MED_COUNT] = {
  { "Lv.10", "Lv.25", "Lv.50", "7 STREAK", "BOND", "TOP FORM", "IN SHAPE",
    "CURED", "SHINY", "LEGEND", "TIRELESS", "LIVELY", "30 STREAK", "CHESS WIN", "CHESS RECORD" },
  { "Lv.10", "Lv.25", "Lv.50", "连续7天", "羁绊", "最终形态", "精神饱满",
    "康复", "异色", "传说", "精力充沛", "活力满满", "连续30天", "棋局胜利", "棋艺新高" },
  { "Lv.10", "Lv.25", "Lv.50", "7일 연속", "유대", "최종 진화", "건강",
    "회복", "색이 다른 모습", "전설", "넘치는 체력", "넘치는 활력", "30일 연속", "체스 승리", "체스 신기록" },
};
static const char *const MED_LBL[LANG_COUNT][MED_COUNT] = {
  { "Lv10", "Lv25", "Lv50", "7DAYS", "BOND", "TOP", "FIT",
    "CURED", "SHINY", "LEGEND", "STA", "VIT", "30DAYS", "WIN", "RECORD" },
  { "Lv10", "Lv25", "Lv50", "7天", "羁绊", "最终", "健康",
    "康复", "异色", "传说", "体力", "活力", "30天", "胜利", "新高" },
  { "Lv10", "Lv25", "Lv50", "7일", "유대", "최종", "건강",
    "회복", "색다름", "전설", "체력", "활력", "30일", "승리", "신기록" },
};
static const char *const MED_DSC[LANG_COUNT][MED_COUNT] = {
  { "LEVEL 10", "LEVEL 25", "LEVEL 50", "7 DAY STREAK", "MAX BOND", "FINAL FORM", "IN SHAPE",
    "GOT CURED", "SHINY FORM", "LEGENDARY", "STAMINA 50", "VITALITY 50",
    "30DAY STREAK", "WON CHESS", "CHESS RECORD" },
  { "等级 10", "等级 25", "等级 50", "连续 7 天", "羁绊 MAX", "最终进化", "精神饱满",
    "吃药治好病", "异色伙伴", "传说宝可梦", "体力练到 50", "活力练到 50",
    "连续 30 天", "陪伴时下棋赢了", "陪伴时棋艺新高" },
  { "레벨 10", "레벨 25", "레벨 50", "7일 연속 돌봄", "최대 유대", "최종 진화형", "건강한 몸",
    "약 먹고 회복", "색이 다른 동료", "전설의 포켓몬", "체력 훈련 50", "활력 훈련 50",
    "30일 연속 돌봄", "함께한 체스 승리", "함께한 체스 신기록" },
};

const char *T(StrId id) { return STRINGS[gLang][id]; }
const char *medalName(int i)  { return MED_NAME[gLang][i]; }
const char *medalLabel(int i) { return MED_LBL[gLang][i]; }
const char *medalDesc(int i)  { return MED_DSC[gLang][i]; }

// La clave es "lang2" y no "lang": con 9 idiomas el ingles era el 1 y el chino
// el 8. Releer esos numeros con la tabla nueva cambiaria el idioma de golpe.
void loadLang() {
  Preferences p;
  p.begin("tamapoke", true);  // solo lectura
  uint8_t v = p.getUChar("lang2", LANG_DEFAULT);
  p.end();
  gLang = (v < LANG_COUNT) ? (Lang)v : LANG_DEFAULT;
}

void setLang(Lang l) {
  if (l >= LANG_COUNT) return;
  gLang = l;
  Preferences p;
  p.begin("tamapoke", false);
  p.putUChar("lang2", (uint8_t)l);
  p.end();
}
