const express = require('express');
const app = express();
const port = 3000;

function generateRandomMovie() {
  const titles = [
    '流浪地球',
    '满江红',
    '独行月球',
    '长津湖',
    '你好，李焕英',
    '我和我的祖国',
    '中国机长',
    '战狼2',
    '哪吒之魔童降世',
    '红海行动'
  ];
  const directors = ['郭帆', '张艺谋', '张吃鱼', '陈凯歌', '贾玲', '陈凯歌', '刘伟强', '吴京', '饺子', '林超贤'];
  const actors = ['吴京', '沈腾', '马丽', '张译', '易烊千玺', '王一博', '刘昊然', '周冬雨', '肖战', '杨紫'];
  const summaries = [
    '在不久的将来，太阳即将毁灭，人类在地球表面建造出巨大的推进器，寻找新的家园。',
    '在一个充满未知的世界里，主人公踏上了一段惊心动魄的冒险之旅。',
    '一个普通人卷入一场惊天阴谋，不得不挺身而出保护他人。',
    '讲述了一群平凡人在非常时期作出的不平凡选择。',
    '一个温暖的故事，展现了人性的光辉和亲情的可贵。'
  ];

  return {
    id: Math.floor(Math.random() * 1000),
    title: titles[Math.floor(Math.random() * titles.length)],
    originalTitle: titles[Math.floor(Math.random() * titles.length)],
    directors: [directors[Math.floor(Math.random() * directors.length)]],
    actors: [actors[Math.floor(Math.random() * actors.length)], actors[Math.floor(Math.random() * actors.length)]],
    rating: (Math.random() * 2 + 7).toFixed(1), // 生成7.0-9.0之间的随机评分
    summary: summaries[Math.floor(Math.random() * summaries.length)]
  };
}

app.get('/api/movies/in_theaters', (req, res) => {
  const movies = Array(10)
    .fill(null)
    .map(() => generateRandomMovie());
  res.json(movies);
});

app.get('/api/movies/top250', (req, res) => {
  const movies = Array(10)
    .fill(null)
    .map(() => generateRandomMovie());
  res.json(movies);
});

app.listen(port, () => {
  console.log(`Mock server is running at http://localhost:${port}`);
});
