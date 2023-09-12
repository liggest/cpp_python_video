from typing import TypeVar, Generic, Callable

InstanceType = TypeVar("InstanceType", bound="SingletonMeta")

class SingletonMeta(type, Generic[InstanceType]):
    """
        元类，使其产生的类变成单例类
        ```Python
            from typing_extensions import Self
            class Test(metaclass=SingletonMeta[Self]): 
                # Self 用于更好的类型推断
                pass
            
            Test.instance is Test.instance # True
            Test() is Test() # True
        ```
        类中可以定义 `staticmethod`、`classmethod` 装饰的 `instance` 方法来覆盖 `instance` 的创建逻辑
        ```Python
            class Test(metaclass=SingletonMeta[Self]):
                _data = None
                @classmethod
                def instance(cls):
                    if cls._data is None:
                        cls._data = super().__new__(cls)
                    return cls._data
        ```
    """
    _instance = None
    
    _overridedCls:dict[type, Callable[[],InstanceType]] = {}

    def __new__(meta, name, bases:list[type], attrs:dict):
        cls = super().__new__(meta, name, bases, attrs)
        if method := attrs.get("instance"):
            # 如果 cls 自己定义了 instance
            if isinstance(method, (staticmethod, classmethod)):
                # 那个 instance 被定义为上述类的对象
                # __get__(None, cls)，得到绑定了 cls 的 instance 方法
                # __get__(None, cls)() 得到装饰函数的返回值
                meta._overridedCls[cls] = method.__get__(None, cls)
        elif any(method := meta._overridedCls.get(base) for base in bases):
            # B 继承于 A，A 定义了 instance，B 没有，那就也用 A 对 instance 的定义
            meta._overridedCls[cls] = method
        return cls


    @property
    def instance(cls:type[InstanceType]) -> InstanceType:
        if method := cls._overridedCls.get(cls):
            # cls 重写了 instance，优先用重写的
            return method()
        if cls._instance is None:
            cls._instance = super().__call__()
        return cls._instance

    def __call__(cls):
        return cls.instance


