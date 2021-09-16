kernel void yunpaozidongji(global int *a,global const int *width,global const int *hight)
{
    int x=get_global_id(0);
    int y=get_global_id(1);
    int xmax=*width;
    int ymax=*hight;
    local int i;
    i=0;
    //a[xmax*y+x]
    //上下左右
        if(x>0)
            i+=a[xmax*y+x-1];
        if (x<(xmax-1))
            i+=a[xmax*y+x+1];
        if (y>0)
            i+=a[xmax*(y-1)+x];
        if (y<(ymax-1))
            i+=a[xmax*(y+1)+x];
    //四个角
        if (x>0&&y>0)
            i+=a[xmax*(y-1)+(x-1)];
        if (y>0&&(x<xmax-1))
            i+=a[xmax*(y-1)+x+1];
        if (y<(ymax-1)&&x>0)
            i+=a[xmax*(y+1)+x-1];
        if (y<(ymax-1)&&(x<xmax-1))
            i+=a[xmax*(y+1)+(x+1)];
    //规则
    if (i<2)
        a[xmax*y+x]=0;
    if (i==2)
        a[xmax*y+x]=a[xmax*y+x]==1?1:0;
    if (i==3)
        a[xmax*y+x]=1;
    if (i>3)
        a[xmax*y+x]=0;
   
}