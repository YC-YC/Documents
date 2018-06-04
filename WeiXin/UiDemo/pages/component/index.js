// pages/component/index.js
Page({

  /**
   * 页面的初始数据
   */
  data: {
    list: [{
      id:"view",
      name:"视图容器",
      open:false,
      pages:['view', 'scroll-vew', 'swiper']
    },
    {
      id: "content",
      name: "基础内容",
      open: false,
      pages: ['text', 'icon', 'progress']
    }]
  },

  kindToggle: function(e){
    var id = e.currentTarget.id, list = this.data.list;
    console.log("click id=" + id);
    for( var i = 0, len = list.length; i < len; i++){
      if (list[i].id == id){
        list[i].open = !list[i].open;
      }
      else{
        list[i].open = false;
      }
    }
    this.setData({
      list: list
    });
  }
})