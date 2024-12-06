#include "../inc/bee_user.h"

gboolean gtk_create_main_window(gpointer user_data) {
    (void)user_data;
    GtkWidget *main_window;
    GtkWidget *main_box;

    /* Вертикальный контейнер для topbar и остального */
    GtkWidget *vertical_box;

    /* Боковая панель */
    GtkWidget *sidebar_box;

    /* Контейнер чата */
    GtkWidget *chat_box;

    /* Верхняя панель чата */
    GtkWidget *topbar_box;
    GtkWidget *avatar_image;
    GdkPixbuf *pixbuf, *scaled_pixbuf;

    /* Поле ввода сообщений */
    GtkWidget *chat_entry_box;
    GtkWidget *chat_entry;

    /* Контейнер для отображения сообщений */
    GtkWidget *message_area;

    /* Создание главного окна */
    main_window = gtk_application_window_new(GTK_APPLICATION(main_data->app));
    gtk_window_set_title(GTK_WINDOW(main_window), "BEE CHAT");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 800);
    gtk_window_set_resizable(GTK_WINDOW(main_window), FALSE); // Фиксированный размер окна

    GtkStyleContext *main_context = gtk_widget_get_style_context(main_window);
    gtk_style_context_add_class(main_context, "main-window");

    /* Основной вертикальный контейнер */
    vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vertical_box);

    /* Верхняя панель чата (topbar_box) */
    topbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(topbar_box, "top_bar");
    gtk_widget_set_size_request(topbar_box, -1, 60);
    gtk_box_pack_start(GTK_BOX(vertical_box), topbar_box, FALSE, FALSE, 0);

    /* Основной горизонтальный контейнер */
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vertical_box), main_box, TRUE, TRUE, 0);

    /* Боковая панель (sidebar_box) */
    sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(sidebar_box, 300, -1); // Фиксированная ширина
    gtk_box_pack_start(GTK_BOX(main_box), sidebar_box, FALSE, TRUE, 0);
    GtkStyleContext *sidebar_context = gtk_widget_get_style_context(sidebar_box);
    gtk_style_context_add_class(sidebar_context, "side-box"); // Добавляем класс для CSS

    /* Добавление аватарки в боковую панель */
    pixbuf = gdk_pixbuf_new_from_file("img/avatar.png", NULL);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 60, 60, GDK_INTERP_BILINEAR); // Масштабирование до 60x60
    avatar_image = gtk_image_new_from_pixbuf(scaled_pixbuf);

    g_object_unref(pixbuf); // Освобождаем память оригинального изображения
    g_object_unref(scaled_pixbuf); // Освобождаем память масштабированного изображения

    gtk_widget_set_halign(avatar_image, GTK_ALIGN_START); // Выравнивание по левому краю
    gtk_box_pack_start(GTK_BOX(topbar_box), avatar_image, FALSE, FALSE, 0); // Добавление сверху

    /* Контейнер для чата */
    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), chat_box, TRUE, TRUE, 0);
    GtkStyleContext *chat_context = gtk_widget_get_style_context(chat_box);
    gtk_style_context_add_class(chat_context, "chat-box"); // Добавляем класс для CSS

    /* Контейнер для отображения сообщений (область истории чата) */
    message_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(message_area, TRUE); // Позволяет растягиваться по вертикали
    gtk_widget_set_hexpand(message_area, TRUE); // Позволяет растягиваться по горизонтали
    gtk_box_pack_start(GTK_BOX(chat_box), message_area, TRUE, TRUE, 0);

    GtkStyleContext *message_area_context = gtk_widget_get_style_context(message_area);
    gtk_style_context_add_class(message_area_context, "message-area"); // Добавляем CSS-класс

    /* Контейнер для ввода сообщений */
    chat_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(chat_entry_box, "chat_entry_box");
    gtk_box_pack_end(GTK_BOX(chat_box), chat_entry_box, FALSE, FALSE, 0);
    //gtk_widget_set_halign(chat_entry_box, GTK_ALIGN_CENTER);

    /* Поле для ввода сообщений */
    chat_entry = gtk_entry_new();
    gtk_widget_set_name(chat_entry, "chat_entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "Type a message...");
    gtk_widget_set_size_request(chat_entry, 500, 40);
    gtk_widget_set_halign(chat_entry, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(chat_entry_box), chat_entry, FALSE, FALSE, 0);
    gtk_widget_set_margin_start(chat_entry, 235);
    /* Отображение всех виджетов */
    gtk_widget_show_all(main_window);

    return G_SOURCE_REMOVE;
}
