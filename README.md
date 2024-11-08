# multi-threaded-file-copy
Bu proje, çok iş parçacıklı (multi-threaded) dosya kopyalama işlevi geliştirmek amacıyla sistem programlama tekniklerini kullanır. Dosya ve dizinleri çok iş parçacıklı bir yapı ile güvenli ve hızlı bir şekilde kopyalamayı hedefler. Proje, dosya sistemi yönetimi, iş parçacığı kullanımı ve işlem süresi analizlerini içerir.

Proje Özellikleri
Çok İş Parçacıklı Yapı: Dosya kopyalama işlemi, birden fazla iş parçacığı kullanılarak hızlandırılmıştır.
Güvenli Eş Zamanlı İşlem: mutex ve atomic türleri kullanılarak çok iş parçacıklı işlemler güvenli hale getirilmiştir.
Performans Analizi: Farklı kopyalama yöntemleri (recursive, for döngüsü) kıyaslanarak işlem süreleri analiz edilmiştir.
Özelleştirilmiş Thread Pool: ThreadPool sınıfı, kopyalama görevlerini eş zamanlı olarak yürütmek için optimize edilmiştir.
Kullanılan Teknolojiler
C++ STL Kütüphaneleri: filesystem, thread, mutex, atomic gibi kütüphanelerle sistem seviyesinde işlemler yapılır.
Performans ve Zaman Analizi: chrono kütüphanesi ile işlem süreleri ölçülerek kullanıcıya geri bildirim sağlanır.
